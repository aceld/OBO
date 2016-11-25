//
// Created by Ace on 2016/11/19.
//

#include "OBOJni.h"
#include "Json.h"
#include "Curl.h"

extern "C" {


JNIEXPORT jboolean JNICALL Java_com_example_ace_obo_OBOJni_Reg
        (JNIEnv *env, jobject obj, jstring jusername, jstring jpasswd,
         jstring jtel, jstring jemail, jboolean jisDriver)
{
    const char *username =  env->GetStringUTFChars(jusername, NULL);
    const char *passwd = env->GetStringUTFChars(jpasswd, NULL);
    const char *tel = env->GetStringUTFChars(jtel, NULL);
    const char *email = env->GetStringUTFChars(jemail, NULL);

    bool isDriver = (jisDriver ==JNI_TRUE) ? true:false;


    JNIINFO("REG: username = %s, passwd = %s, tel = %s, email =%s, isDriver = %d",
            username, passwd, tel, email, isDriver);



    /*
     * 给服务端的协议   https://ip:port/reg [json_data]
     *
    {
        username: "aaa",
        password: "bbb",
        driver:   "yes/no",
        tel:      "13331333333",
        email:    "danbing_at@163.cn"
    }
     */
    Json json;

    json.insert("username", username);
    json.insert("password", passwd);
    json.insert("driver", isDriver?"yes":"no");
    json.insert("tel", tel);
    json.insert("email", email);

    string json_str = json.print();


    //释放字符串资源
    env->ReleaseStringUTFChars(jusername, username);
    env->ReleaseStringUTFChars(jpasswd, passwd);
    env->ReleaseStringUTFChars(jtel, tel);
    env->ReleaseStringUTFChars(jemail, email);

    string url = OBO_SERVER_IP;
    url +=":";
    url +=OBO_SERVER_PORT;
    url +="/reg";



    Curl curl(url, true);


    if (!curl.execute(json_str)) {
        return JNI_FALSE;
    }


    /*
     *  得到服务器响应数据, 注册成功就默认为登陆状态

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

    string response_data = curl.responseData();
    Json json_response;
    json_response.parse(curl.responseData());

    string result = json.value("result");
    if (result.length() != 0) {
        if (result == "ok") {

            string sessionid = json.value("sessionid");
            JNIINFO("reg succ, sessionid=%s", sessionid.c_str());
            return JNI_TRUE;

        }
        else {
            JNIINFO("ret error data= %s", response_data.c_str());
            return JNI_FALSE;
        }
    }




    return JNI_TRUE;
}

}