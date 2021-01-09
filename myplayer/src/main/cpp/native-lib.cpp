#include <jni.h>
#include <string>
#include "WlFFmpeg.h"

extern "C"
{
#include <libavformat/avformat.h>
}


_JavaVM *javaVM = NULL;
WlCallJava *callJava = NULL;
WlFFmpeg *fFmpeg = NULL;
WlPlaystatus *playstatus = NULL;

bool nexit = true;
pthread_t thread_start;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if(vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        return result;
    }
    return JNI_VERSION_1_4;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1parpared(JNIEnv *env, jobject instance,
                                                      jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if(fFmpeg == NULL)
    {
        if(callJava == NULL)
        {
            callJava = new WlCallJava(javaVM, env, &instance);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playstatus = new WlPlaystatus();
        fFmpeg = new WlFFmpeg(playstatus,callJava, source);
        fFmpeg->parpared();
    }
}

void *startCallBack(void *data)
{
    WlFFmpeg *fFmpeg = (WlFFmpeg *) data;
    fFmpeg->start();
    pthread_exit(&thread_start);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1start(JNIEnv *env, jobject instance) {

    // TODO
    if(fFmpeg != NULL)
    {
        pthread_create(&thread_start, NULL, startCallBack, fFmpeg);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1pause(JNIEnv *env, jobject instance) {

    // TODO
    if(fFmpeg != NULL)
    {
        fFmpeg->pause();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1resume(JNIEnv *env, jobject instance) {

    // TODO
    if(fFmpeg != NULL)
    {
        fFmpeg->resume();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1stop(JNIEnv *env, jobject instance) {

    // TODO

    if(!nexit)
    {
        return;
    }
    jclass jlz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(jlz, "onCallNext", "()V");
    nexit = false;
    if(fFmpeg != NULL)
    {
        fFmpeg->release();
        delete(fFmpeg);
        fFmpeg = NULL;
        if(callJava != NULL)
        {
            delete(callJava);
            callJava = NULL;
        }
        if(playstatus != NULL)
        {
            delete(playstatus);
            playstatus = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(instance, jmid_next);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1seek(JNIEnv *env, jobject thiz, jint secds) {
    // TODO: implement n_seek()
    if(fFmpeg != NULL)
    {
        LOGE("xll Java_com_wyp_myplayer_player_WlPlayer_n_1seek %d",secds);
        fFmpeg->seek(secds);
    }
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1duration(JNIEnv *env, jobject thiz) {
    // TODO: implement n_duration()
    if(fFmpeg != NULL)
    {
        return fFmpeg->duration;
    }
    return -1;
}extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1volume(JNIEnv *env, jobject instance, jint percent) {

    if(fFmpeg!=NULL){
        fFmpeg->setVolume(percent);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1pitch(JNIEnv *env, jobject instance, jfloat pitch) {

    // TODO
    if(fFmpeg!=NULL){
        fFmpeg->setPitch(pitch);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_wyp_myplayer_player_WlPlayer_n_1speed(JNIEnv *env, jobject instance, jfloat speed) {

    // TODO
    if(fFmpeg!=NULL){
        fFmpeg->setSpeed(speed);
    }

}