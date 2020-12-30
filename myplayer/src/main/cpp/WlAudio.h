//
// Created by yangw on 2018-2-28.
//

#ifndef MYMUSIC_WLAUDIO_H
#define MYMUSIC_WLAUDIO_H

#include "WlQueue.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};

class WlAudio {

public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    WlQueue * queue =NULL;
    WlPlaystatus * playstatus = NULL;

public:
    WlAudio(WlPlaystatus* wlPlaystatus);
    ~WlAudio();

};


#endif //MYMUSIC_WLAUDIO_H
