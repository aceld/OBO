//
// Created by Ace on 2016/11/19.
//

#ifndef OBO_OBOJNI_H
#define OBO_OBOJNI_H


#include <stdio.h>
#include <string>
#include <jni.h>
#include <android/log.h>

using namespace std;

#define jniLogTag           "JNI"
#define OBO_SERVER_IP       "https://101.200.190.150"
#define OBO_SERVER_PORT     "18888"

#define RESPONSE_DATA_LEN   (4096)
#define TIME_STR_LEN        (64)

#define JNIINFO(fmt, ...) \
    __android_log_print(ANDROID_LOG_INFO, jniLogTag, fmt, __VA_ARGS__);




#endif //OBO_OBOJNI_H
