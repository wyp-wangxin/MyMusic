//
// Created by yangw on 2018-2-28.
//

#ifndef MYMUSIC_WLCALLJAVA_H
#define MYMUSIC_WLCALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>
#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1


class WlCallJava {

public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_parpared;

public:
    WlCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj);
    ~WlCallJava();

    void onCallParpared(int type);

};


#endif //MYMUSIC_WLCALLJAVA_H
