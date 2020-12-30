//
// Created by yangw on 2018-2-28.
//

#include "WlFFmpeg.h"

WlFFmpeg::WlFFmpeg(WlPlaystatus *playstatus,WlCallJava *callJava, const char *url) {
    this->playstatus=playstatus;
    this->callJava = callJava;
    this->url = url;
}

void *decodeFFmpeg(void *data)
{
    WlFFmpeg *wlFFmpeg = (WlFFmpeg *) data;
    wlFFmpeg->decodeFFmpegThread();
    pthread_exit(&wlFFmpeg->decodeThread);
}

void WlFFmpeg::parpared() {

    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);

}

void WlFFmpeg::decodeFFmpegThread() {

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    if(avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not open url :%s", url);
        }
        return;
    }
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not find streams from %s", url);
        }
        return;
    }
    for(int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)//得到音频流
        {
            if(audio == NULL)
            {
                audio = new WlAudio(playstatus);
                audio->streamIndex = i;
                audio->codecpar = pFormatCtx->streams[i]->codecpar;
            }
        }
    }

    AVCodec *dec = avcodec_find_decoder(audio->codecpar->codec_id);
    if(!dec)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not find decoder");
        }
        return;
    }

    audio->avCodecContext = avcodec_alloc_context3(dec);
    if(!audio->avCodecContext)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not alloc new decodecctx");
        }
        return;
    }

    if(avcodec_parameters_to_context(audio->avCodecContext, audio->codecpar) < 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("can not fill decodecctx");
        }
        return;
    }

    if(avcodec_open2(audio->avCodecContext, dec, 0) != 0)
    {
        if(LOG_DEBUG)
        {
            LOGE("cant not open audio strames");
        }
        return;
    }
    callJava->onCallParpared(CHILD_THREAD);
}

void WlFFmpeg::start() {

    if(audio == NULL)
    {
        if(LOG_DEBUG)
        {
            LOGE("audio is null");
            return;
        }
    }

    int count = 0;

    while(1)
    {
        AVPacket *avPacket = av_packet_alloc();
        if(av_read_frame(pFormatCtx, avPacket) == 0)
        {
            if(avPacket->stream_index == audio->streamIndex)
            {
                //解码操作
                count++;
                if(LOG_DEBUG)
                {
                    LOGE("解码第 %d 帧", count);
                }
                /*av_packet_free(&avPacket);
                av_free(avPacket);*/
                audio->queue->putAvpacket(avPacket);

            } else{
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else{
            if(LOG_DEBUG)
            {
                LOGE("decode finished");
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            break;
        }
    }
    //模拟出队
    while (audio->queue->getQueueSize() > 0)
    {
        AVPacket *packet = av_packet_alloc();
        audio->queue->getAvpacket(packet);
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    if(LOG_DEBUG)
    {
        LOGD("解码完成");
    }
}
