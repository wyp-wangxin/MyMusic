//
// Created by yangw on 2018-2-28.
//

#include "WlCallJava.h"

WlCallJava::WlCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {

    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass  jlz = jniEnv->GetObjectClass(jobj);
    if(!jlz)
    {
        if(LOG_DEBUG)
        {
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_parpared = env->GetMethodID(jlz, "onCallParpared", "()V");




}

void WlCallJava::onCallParpared(int type) {

    if(type == MAIN_THREAD)
    {
        jniEnv->CallVoidMethod(jobj, jmid_parpared);
    }
    else if(type == CHILD_THREAD)
    {
        JNIEnv *jniEnv;
        if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
        {
            if(LOG_DEBUG)
            {
                LOGE("get child thread jnienv worng");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_parpared);
        javaVM->DetachCurrentThread();
    }

}
