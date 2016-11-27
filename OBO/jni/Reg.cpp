//
// Created by Ace on 2016/11/19.
//

#include "OBOJni.h"
#include "Json.h"
#include "Curl.h"

extern "C" {


JNIEXPORT jboolean JNICALL Java_com_example_ace_obo_OBOJni_Reg
        (JNIEnv *env, jobject obj, jstring jusername, jstring jpasswd,
         jstring jtel, jstring jemail, jstring jid_card, jboolean jisDriver)
{


    const char *username =  env->GetStringUTFChars(jusername, NULL);
    const char *passwd = env->GetStringUTFChars(jpasswd, NULL);
    const char *tel = env->GetStringUTFChars(jtel, NULL);
    const char *email = env->GetStringUTFChars(jemail, NULL);
    const char *id_card = env->GetStringUTFChars(jid_card, NULL);


    bool isDriver = (jisDriver ==JNI_TRUE) ? true:false;


    JNIINFO("REG: username = %s, passwd = %s, tel = %s, email =%s, idcard = %s,isDriver = %d",
            username, passwd, tel, email, id_card, isDriver);



    /*
     * 给服务端的协议   https://ip:port/reg [json_data]
     *
    {
        username: "aaa",
        password: "bbb",
        driver:   "yes/no",
        tel:      "13331333333",
        email:    "danbing_at@163.cn",
        id_card:  "2104041222121211122"
    }
     */
    Json json;

    json.insert("username", username);
    json.insert("password", passwd);
    json.insert("driver", isDriver?"yes":"no");
    json.insert("tel", tel);
    json.insert("email", email);
    json.insert("id_card", id_card);

    string json_str = json.print();


    //释放字符串资源
    env->ReleaseStringUTFChars(jusername, username);
    env->ReleaseStringUTFChars(jpasswd, passwd);
    env->ReleaseStringUTFChars(jtel, tel);
    env->ReleaseStringUTFChars(jemail, email);
    env->ReleaseStringUTFChars(jid_card, id_card);


    string url = OBO_SERVER_IP;
    url +=":";
    url +=OBO_SERVER_PORT;
    url +="/reg";



    Curl curl(url, true);


    if (curl.execute(json_str) == false) {
        JNIINFO("%s", "curl execute error")
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

    string result = json_response.value("result");
    if (result.length() != 0) {
        if (result == "ok") {

            g_session = json_response.value("sessionid");
            JNIINFO("reg succ, sessionid=%s", g_session.c_str());
            return JNI_TRUE;

        }
        else {
            JNIINFO("ret error data= %s", response_data.c_str());
            return JNI_FALSE;
        }
    }

    JNIINFO("result len =%d", result.length());


    return JNI_FALSE;
}

}