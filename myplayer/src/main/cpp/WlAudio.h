//
// Created by yangw on 2018-2-28.
//

#ifndef MYMUSIC_WLAUDIO_H
#define MYMUSIC_WLAUDIO_H

#include "WlQueue.h"
#include "pthread.h"
#include "AndroidLog.h"
#include "WlCallJava.h"
#include "SoundTouch.h"
extern "C"
{
#include <libavutil/time.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};
using namespace soundtouch;
class WlAudio {

public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = NULL;
    AVCodecParameters *codecpar = NULL;
    WlQueue * queue =NULL;
    WlPlaystatus * playstatus = NULL;
    WlCallJava * wlCallJava=NULL;

    pthread_t thread_play;

    uint8_t *buffer = NULL;
    int data_size;
    int sample_rate;

    int duration = 0;
    AVRational time_base;
    double clock;//总的播放时长
    double now_time;//当前frame时间
    double last_tiem; //上一次调用时间

    int volumePercent = 100;//默认音量
    int mute = 2;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;
    SLVolumeItf pcmVolumePlay = NULL;//用于音量控制相关
    SLMuteSoloItf  pcmMutePlay = NULL;//用于声道设置
    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    //SoundTouch
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    bool finished = true;
    uint8_t *out_buffer = NULL;
    int nb = 0;
    int num = 0;

    float pitch = 1.0f;//默認音調
    float speed = 1.0f;//默認音樂速度
    bool isRecordPcm = false ;

public:
    WlAudio(WlPlaystatus* wlPlaystatus,int sample_rate,WlCallJava * wlCallJava);
    ~WlAudio();

    void play();

    int resampleAudio(void **pcmbuf);
    void initOpenSLES();
    int getCurrentSampleRateForOpensles(int sample_rate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);//设置音量

    int getSoundTouchData();//SoundTouch 處理PCM數據

    void setMute(int mute);

    void setPitch(float pitch);//設置音調

    void setSpeed(float speed);//設置音速

    int getPCMDB(char *pcmcata, size_t pcmsize);

    void startStopRecord(bool start);
};


#endif //MYMUSIC_WLAUDIO_H
