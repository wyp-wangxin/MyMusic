//
// Created by yangw on 2018-2-28.
//

#include "WlAudio.h"



WlAudio::~WlAudio() {

}

WlAudio::WlAudio(WlPlaystatus *wlPlaystatus,int sample_rate,WlCallJava * wlCallJava) {
    this->playstatus = wlPlaystatus;
    this->sample_rate = sample_rate;
    this->wlCallJava=wlCallJava;
    queue = new WlQueue(playstatus);
    buffer = static_cast<uint8_t *>(av_malloc(sample_rate * 2 * 2));

    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));//分配空间大小
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);
}

void * decodPlay(void * data){
    WlAudio *wlAudio = (WlAudio *) data;

    //wlAudio->resampleAudio();
    wlAudio->initOpenSLES();
    pthread_exit(&wlAudio->thread_play);
}

void WlAudio::play() {
    pthread_create(&thread_play,NULL, decodPlay, this);
}

int WlAudio::resampleAudio(void **pcmbuf) {
    int ret =0;

    while (playstatus!=NULL&&!playstatus->exit){
        if(queue->getQueueSize() == 0)//加载中
        {
            if(!playstatus->load)
            {
                playstatus->load = true;
                wlCallJava->onCallLoad(CHILD_THREAD, true);
            }
            continue;
        } else{
            if(playstatus->load)
            {
                playstatus->load = false;
                wlCallJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        AVPacket *avPacket = av_packet_alloc();
        if(queue->getAvpacket(avPacket)!=0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket=NULL;
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if(ret != 0)
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        AVFrame * avFrame = av_frame_alloc();
        ret= avcodec_receive_frame(avCodecContext,avFrame);
        if(ret==0){
            if(avFrame->channels>0&&avFrame->channel_layout == 0){
                avFrame->channel_layout=av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels ==0&&avFrame->channel_layout >0){
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swrContext = swr_alloc_set_opts(NULL,
                                                        AV_CH_LAYOUT_STEREO,
                                                        AV_SAMPLE_FMT_S16,
                                                        avFrame->sample_rate,
                                                        avFrame->channel_layout,
                                                        (AVSampleFormat)(avFrame->format),
                                                        avFrame->sample_rate,
                                                        NULL,NULL);
            if(!swrContext || swr_init(swrContext)<0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swrContext);
                continue;
            }

            nb = swr_convert(swrContext,
                                 &buffer,
                                 avFrame->nb_samples,
                                 (const uint8_t **)(avFrame->data),
                                 avFrame->nb_samples);


            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            //ret = fwrite(buffer,1,data_size,outFile);
            //LOGE("data_size is %d,ret=%d", data_size,ret);

            *pcmbuf = buffer;//數據保存給pcmbuf，在getSoundTouchData裡面會使用
            now_time = avFrame->pts * av_q2d(time_base);
            if(now_time < clock)
            {
                now_time = clock;
            }
            clock = now_time;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);

            break;
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame=NULL;
            continue;
        }

    }
    //fflush(outFile);
    //LOGE("outFile is fflush");
    return data_size;
}


int WlAudio::getSoundTouchData() {
    while(playstatus != NULL && !playstatus->exit)
    {
        out_buffer = NULL;
        if(finished)
        {
            finished = false;
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
            if(data_size > 0)
            {
                for(int i = 0; i < data_size / 2 + 1; i++)
                {
                    sampleBuffer[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleBuffer, nb);
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
            } else{
                soundTouch->flush();//处理缓存。
            }
        }
        if(num == 0)
        {
            finished = true;
            continue;
        } else{
            if(out_buffer == NULL)
            {
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if(num == 0)
                {
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}


void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
{
    WlAudio *wlAudio = (WlAudio *) context;
    if(wlAudio != NULL)
    {
        int buffersize = wlAudio->getSoundTouchData();
        if(buffersize > 0)
        {
            wlAudio->clock += buffersize / ((double)(wlAudio->sample_rate * 2 * 2));
            if(wlAudio->clock - wlAudio->last_tiem >= 0.1)
            {
                wlAudio->last_tiem = wlAudio->clock;
                //回调应用层
                wlAudio->wlCallJava->onCallTimeInfo(CHILD_THREAD, wlAudio->clock, wlAudio->duration);
            }

            if(wlAudio->isRecordPcm)
            {
                LOGD("start recorder ！！");
                wlAudio->wlCallJava->onCallPcmToAAc(CHILD_THREAD, buffersize * 2 * 2, wlAudio->sampleBuffer);
            }
            wlAudio->wlCallJava->onCallValumeDB(CHILD_THREAD,
            wlAudio->getPCMDB(reinterpret_cast<char *>(wlAudio->sampleBuffer), buffersize * 4));
            (* wlAudio-> pcmBufferQueue)->Enqueue( wlAudio->pcmBufferQueue, (char *) wlAudio-> sampleBuffer, buffersize*2*2);
        }
    }
}

void WlAudio::initOpenSLES() {

    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            (SLuint32)getCurrentSampleRateForOpensles(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 3, ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);//获取音量控制接口pcmVolumePlay


    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);  //获取声道接口

//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);

}
int WlAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate)
    {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate =  SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void WlAudio::pause() {
    if(pcmPlayerPlay != NULL)
    {

        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_PAUSED);
    }
}

void WlAudio::resume() {
    if(pcmPlayerPlay != NULL)
    {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_PLAYING);
    }
}

void WlAudio::stop() {
    if(pcmPlayerPlay != NULL)
    {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay,  SL_PLAYSTATE_STOPPED);
    }
}

void WlAudio::release() {//回收资源
    stop();
    if(queue != NULL)
    {
        delete(queue);
        queue = NULL;
    }

    if(pcmPlayerObject != NULL)
    {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }

    if(outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if(engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if(buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }

    if(avCodecContext != NULL)
    {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    if(playstatus != NULL)
    {
        playstatus = NULL;
    }
    if(wlCallJava != NULL)
    {
        wlCallJava = NULL;
    }
}

void WlAudio::setVolume(int percent) {
    volumePercent = percent;
    if(pcmVolumePlay != NULL)
    {
        if(percent > 30)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        }
        else if(percent > 25)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        }
        else if(percent > 20)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        }
        else if(percent > 15)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        }
        else if(percent > 10)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        }
        else if(percent > 5)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        }
        else if(percent > 3)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        }
        else if(percent > 0)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        }
        else{
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void WlAudio::setPitch(float pitch) {
    this->pitch = pitch;
    if(soundTouch != NULL)
    {
        soundTouch->setPitch(pitch);
    }
}

void WlAudio::setSpeed(float speed) {
    this->speed = speed;
    if(soundTouch != NULL)
    {
        soundTouch->setTempo(speed);
    }
}

void WlAudio::setMute(int mute) {
    this->mute = mute;
    if(pcmMutePlay != NULL)
    {
        if(mute == 0)//right
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
        }
        else if(mute == 1)//left
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }
        else if(mute == 2)//center
        {
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
        }


    }
}

int WlAudio::getPCMDB(char *pcmcata, size_t pcmsize) {
    int db = 0;
    short int pervalue = 0;
    double sum = 0;
    for(int i = 0; i < pcmsize; i+= 2)
    {
        memcpy(&pervalue, pcmcata+i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (pcmsize / 2);
    if(sum > 0)
    {
        db = (int)20.0 *log10(sum);
    }
    return db;
}

void WlAudio::startStopRecord(bool start) {
    this->isRecordPcm = start;

}


