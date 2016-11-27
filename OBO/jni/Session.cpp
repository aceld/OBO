//
// Created by Ace on 2016/11/26.
//
#include "OBOJni.h"


//全局sessionid
string g_session;

extern "C" {

JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getSession
        (JNIEnv *env, jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(g_session.c_str());

    return retstr;

}


}
