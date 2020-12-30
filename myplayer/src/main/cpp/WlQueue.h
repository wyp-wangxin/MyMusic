//
// Created by wyp on 2020/12/30.
//

#ifndef MYMUSIC_WLQUEUE_H
#define MYMUSIC_WLQUEUE_H
#include "queue"
#include "pthread.h"
#include "WlPlaystatus.h"

extern "C"
{
#include <libavcodec/avcodec.h>
};

class WlQueue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    WlPlaystatus *playstatus =NULL;

    WlQueue(WlPlaystatus*wlPlaystatus);
    ~WlQueue();

    int putAvpacket(AVPacket* packet);

    void getAvpacket(AVPacket* packet);

    int getQueueSize();
};


#endif //MYMUSIC_WLQUEUE_H
