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

JNIEXPORT void JNICALL Java_com_example_ace_obo_OBOJni_setStatus
        (JNIEnv *env, jobject obj, jstring jstatus)

{
    const char *status = env->GetStringUTFChars(jstatus, NULL);

    Data::getInstance()->setStatus(status);

    env->ReleaseStringUTFChars(jstatus, status);
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


JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getPtempLongitude
        (JNIEnv *env, jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->get_ptemp_longitude().c_str());

    return retstr;
}


JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getPtempLatitude
        (JNIEnv *env , jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->get_ptemp_latitude().c_str());

    return retstr;
}



JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getDtempLongitude
        (JNIEnv *env, jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->get_dtemp_longitude().c_str());

    return retstr;
}


JNIEXPORT jstring JNICALL Java_com_example_ace_obo_OBOJni_getDtempLatitude
        (JNIEnv *env, jobject obj)
{
    //将char* 转换成jstring，供java使用
    jstring retstr = env->NewStringUTF(Data::getInstance()->get_dtemp_latitude().c_str());

    return retstr;
}


}
