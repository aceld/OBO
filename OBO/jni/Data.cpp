//
// Created by Ace on 2016/11/26.
//
#include "OBOJni.h"
#include "Data.h"

Data *Data::instance = NULL;


extern "C" {

JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getOrderid
        (JNIEnv *env, jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->getOrderid().c_str());

    return retstr;
}

JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getSessionid
        (JNIEnv *env, jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->getSessionid().c_str());

    return retstr;
}

JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getStatus
        (JNIEnv *env, jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->getStatus().c_str());

    return retstr;
}


JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getIsDriver
        (JNIEnv *env , jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->getIsDriver().c_str());

    return retstr;
}



}
