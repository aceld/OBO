//
// Created by Ace on 2016/11/18.
//

#include "OBOJni.h"
#include "curl/curl.h"
#include "cJSON.h"
#include "Data.h"


typedef struct login_response_data
{

    login_response_data() {
        memset(data, 0, RESPONSE_DATA_LEN);
        data_len = 0;
    }

    char data[RESPONSE_DATA_LEN];
    int data_len;

}login_response_data_t;

size_t curl_login_callback(char* ptr, size_t n, size_t m, void* userdata)
{
    login_response_data_t *data = (login_response_data_t*)userdata;
    int response_data_len = n*m;

    memcpy(data->data+data->data_len, ptr, response_data_len);
    data->data_len+=response_data_len;

    return response_data_len;
}



extern "C" {

/*
 * Class:     com_example_ace_obo_Jni
 * Method:    Login
 * Signature: ()I
 */
JNIEXPORT jboolean JNICALL Java_com_example_ace_obo_OBOJni_Login
        (JNIEnv *env, jobject obj, jstring jusername, jstring jpasswd, jboolean jisDriver)
{

    const char *username =  env->GetStringUTFChars(jusername, NULL);
    const char *passwd = env->GetStringUTFChars(jpasswd, NULL);
    const char *isDriver = (jisDriver==JNI_TRUE)?"yes":"no";

    JNIINFO("LOGIN: username = %s, passwd = %s, isDriver = %s", username, passwd, isDriver);

    //保存当前角色
    Data::getInstance()->setIsDriver(isDriver);
    //设置当前的orderid为空
    Data::getInstance()->setOrderid("NONE");
    //更新当前角色状态
    if (jisDriver == JNI_TRUE) {
        Data::getInstance()->setStatus(OBO_STATUS_DRIVER_IDLE);
    }
    else {
        Data::getInstance()->setStatus(OBO_STATUS_PASSNGER_IDLE);
    }

    /*
     * 给服务端的协议   https://ip:port/login [json_data]
     *
    {
        username: "aaa",
        password: "bbb",
        driver: "yes"
    }

     * 得到服务器响应数据

    //成功
    {
        result: "ok",
        sessionid: "xxxxxxxx"
    }
    //失败
    {
        result: "error",
        reason: "why...."
    }

     */

    //封装登陆json字符串
    cJSON *root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "username", username);
    cJSON_AddStringToObject(root, "password", passwd);
    cJSON_AddStringToObject(root, "driver", isDriver);

    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);



    //发送url请求，
    CURLcode curl_ret;
    CURL *curl = curl_easy_init();

    login_response_data_t res_data;
    const char *uri = "https://101.200.190.150:18888/login";


    curl_easy_setopt(curl, CURLOPT_URL, uri);
    curl_easy_setopt(curl, CURLOPT_POST, true);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);

    /*
     * 如果用HTTPS协议，额外需要添加的部分
     * */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_login_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res_data);

    curl_ret = curl_easy_perform(curl);
    if (curl_ret != CURLE_OK) {
        JNIINFO("curl perform error %s\n%s", uri, json_str);
        return JNI_FALSE;
    }
    curl_easy_cleanup(curl);

    free(json_str);



    //得到服务器响应数据
    /*
     *
       //成功
       {
            result: "ok",
            recode: "0",
            sessionid: "xxxxxxxx"
        }
        //失败
        {
            result: "error",
            reason: "why...."
        }

     */
    res_data.data[res_data.data_len] = '\0';
    bool login_succ = false;

    root = cJSON_Parse(res_data.data);
    cJSON * result = cJSON_GetObjectItem(root, "result");
    if (result && (strcmp(result->valuestring, "ok")==0) ) {
        //succ
        Data::getInstance()->setSessionid(cJSON_GetObjectItem(root, "sessionid")->valuestring);
        JNIINFO("login succ, sessionid=%s", Data::getInstance()->getSessionid().c_str());
        login_succ = true;


    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(root, "reason");
        if (reason) {
            JNIINFO("Login error, reason = %s", reason->valuestring);
        }
        else {
            JNIINFO("Login error, unknow reason, res_data=%s", res_data.data);
        }

        login_succ = false;
    }

    cJSON_Delete(root);


    //释放字符串资源
    env->ReleaseStringUTFChars(jusername, username);
    env->ReleaseStringUTFChars(jpasswd, passwd);

    if (login_succ == true) {
        return JNI_TRUE;
    }
    else {
        return JNI_FALSE;
    }
}



}
