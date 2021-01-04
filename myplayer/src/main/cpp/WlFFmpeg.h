//
// Created by yangw on 2018-2-28.
//

#ifndef MYMUSIC_WLFFMPEG_H
#define MYMUSIC_WLFFMPEG_H

#include "WlCallJava.h"
#include "pthread.h"
#include "WlAudio.h"

extern "C"
{
#include "libavformat/avformat.h"
#include <libavutil/time.h>
};


class WlFFmpeg {

public:
    WlCallJava *callJava = NULL;
    const char* url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
    WlAudio *audio = NULL;
    WlPlaystatus *playstatus = NULL;
    pthread_mutex_t init_mutex;
    bool exit = false;



public:
    WlFFmpeg(WlPlaystatus *playstatus,WlCallJava *callJava, const char *url);
    ~WlFFmpeg();

    void parpared();
    void decodeFFmpegThread();
    void start();
    void pause();
    void resume();
    void release();

};


#endif //MYMUSIC_WLFFMPEG_H
