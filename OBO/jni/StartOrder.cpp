//
// Created by Ace on 2016/12/1.
//
#include "OBOJni.h"
#include "Json.h"
#include "Curl.h"
#include "Data.h"


void get_time_str( char* time_str )
{
    time_t t = time(NULL);
    strftime( time_str, TIME_STR_LEN, "%Y-%m-%d %X",localtime(&t) );
}

extern "C" {


/*
 * Class:     com_example_ace_obo_OBOJni
 * Method:    StartOrder
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_ace_obo_OBOJni_StartOrder
        (JNIEnv *env , jobject obj ,
         jstring jsrc_longitude, jstring jsrc_latitude, jstring jsrc_address,
         jstring jdst_longitude, jstring jdst_latitude, jstring jdst_address,
         jstring jRMB)
{
    const char *src_longitude =  env->GetStringUTFChars(jsrc_longitude, NULL);
    const char *src_latitude = env->GetStringUTFChars(jsrc_latitude, NULL);
    const char *src_address = env->GetStringUTFChars(jsrc_address, NULL);
    const char *dst_longitude = env->GetStringUTFChars(jdst_longitude, NULL);
    const char *dst_latitude = env->GetStringUTFChars(jdst_latitude, NULL);
    const char *dst_address = env->GetStringUTFChars(jdst_address, NULL);
    const char *RMB  = env->GetStringUTFChars(jRMB, NULL);

    /*
     ==== 给服务端的协议 ====
    https://ip:port/startSetOrder  [json_data]
    {
        sessionid: "online-user-xxxxxx-xxx-xxx-xxx-xxxxx",
        driver:    "no",
        src_longitude: "108.123123123",//乘客当前经度
        src_latitude : "39.123123123", //乘客当前纬度
        dst_longitude: "90.123123123", //乘客目标经度
        dst_latitude:  "39.123123123",  //乘客目标纬度
        src_address:   "北京西三旗百汇城",
        dst_address:   "北京西二旗软件园9号楼",
        RMB:          "315￥",         //本单大约消费
        create_order_time:"2016-11-12 12:00:00" //创建订单时间
    }


   */
    Json json;

    json.insert("sessionid", Data::getInstance()->getSessionid().c_str());
    json.insert("driver", "no");
    json.insert("src_longitude", src_longitude);
    json.insert("src_latitude", src_latitude);
    json.insert("dst_longitude", dst_longitude);
    json.insert("dst_latitude", dst_latitude);
    json.insert("src_address", src_address);
    json.insert("dst_address", dst_address);
    json.insert("RMB", RMB);
    char now_time[TIME_STR_LEN] = {0};
    get_time_str(now_time);
    json.insert("create_order_time", now_time);


    string json_str = json.print();



    //释放字符串资源
    env->ReleaseStringUTFChars(jsrc_longitude, src_longitude);
    env->ReleaseStringUTFChars(jsrc_latitude, src_latitude);
    env->ReleaseStringUTFChars(jsrc_address, src_address);
    env->ReleaseStringUTFChars(jdst_longitude, dst_longitude);
    env->ReleaseStringUTFChars(jdst_latitude, dst_latitude);
    env->ReleaseStringUTFChars(jdst_address, dst_address);
    env->ReleaseStringUTFChars(jRMB, RMB);



    string url = OBO_SERVER_IP;
    url +=":";
    url +=OBO_SERVER_PORT;
    url +="/startSetOrder";

    Curl curl(url, true);


    if (curl.execute(json_str) == false) {
        JNIINFO("%s", "curl execute error")
        return JNI_FALSE;
    }

    /*
  ====得到服务器响应数据 ====
  //成功
  {
         result: "ok",
         recode: "0",
         orderid: "orderid-xxxx-xxx-xxx-xxx-xxxx"
  }
  //失败
  {
         result: "error",
         recode: "1", //1 代表session失效，需要重新登录
                      //2 代表服务器发生错误
                      //3 附近没有司机可用
         reason: "why...."
  }
  * */


    string response_data = curl.responseData();
    Json json_response;
    json_response.parse(curl.responseData());

    string result = json_response.value("result");
    if (result.length() != 0) {
        if (result == "ok") {

            string recode = json_response.value("recode");
            if (recode == "0") {
                Data::getInstance()->setOrderid(json_response.value("ordierid"));
                JNIINFO("reg succ, orderid=%s", Data::getInstance()->getOrderid().c_str());
                return JNI_TRUE;
            }
            else if (recode == "3") {
                JNIINFO("%s", "waiting driver...");
                return JNI_FALSE;
            }

        }
        else {
            JNIINFO("ret error data= %s", response_data.c_str());
            return JNI_FALSE;
        }
    }


    return JNI_TRUE;
}






};