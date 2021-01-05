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

int WlAudio::resampleAudio() {
    int ret =0;
    /*char *url="/data/user/0/com.wyp.mymusic/cache/mydream.pcm";
    FILE *outFile = fopen(url, "wb+");
    if (!outFile){
        LOGE("can not open file : %s",url);
        return -1;
    }*/
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

            int nb = swr_convert(swrContext,
                                 &buffer,
                                 avFrame->nb_samples,
                                 (const uint8_t **)(avFrame->data),
                                 avFrame->nb_samples);


            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            //ret = fwrite(buffer,1,data_size,outFile);
            //LOGE("data_size is %d,ret=%d", data_size,ret);

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

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void * context)
{
    WlAudio *wlAudio = (WlAudio *) context;
    if(wlAudio != NULL)
    {
        int buffersize = wlAudio->resampleAudio();
        if(buffersize > 0)
        {
            wlAudio->clock += buffersize / ((double)(wlAudio->sample_rate * 2 * 2));
            if(wlAudio->clock - wlAudio->last_tiem >= 0.1)
            {
                wlAudio->last_tiem = wlAudio->clock;
                //回调应用层
                wlAudio->wlCallJava->onCallTimeInfo(CHILD_THREAD, wlAudio->clock, wlAudio->duration);
            }
            (* wlAudio-> pcmBufferQueue)->Enqueue( wlAudio->pcmBufferQueue, (char *) wlAudio-> buffer, buffersize);
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


    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 1, ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

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
