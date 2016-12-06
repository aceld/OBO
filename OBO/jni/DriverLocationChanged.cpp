//
// Created by Ace on 2016/12/5.
//
#include "OBOJni.h"
#include "Json.h"
#include "Curl.h"
#include "Data.h"

extern "C" {

JNIEXPORT jboolean JNICALL Java_com_example_ace_obo_OBOJni_DriverLocationChanged
        (JNIEnv *env, jobject obj , jstring jlongitude, jstring jlatitude, jstring jaddress, jstring jautoSend)
{
    const char *longitude =  env->GetStringUTFChars(jlongitude, NULL);
    const char *latitude = env->GetStringUTFChars(jlatitude, NULL);
    const char *address = env->GetStringUTFChars(jaddress, NULL);
    const char *autoSend = env->GetStringUTFChars(jautoSend, NULL);

/*
 *     ==== 给服务端的协议 ====
    https://ip:port/locationChanged  [json_data]
    {
        sessionid: "online-driver-xxxxxx-xxx-xxx-xxx-xxxxx",
        driver:    "yes",
        status:    "ready",//当前司机端状态
        longitude: "188.123123123",//当前经度
        latitude : "39.123123123", //当前纬度
        address: "北京市西三旗金燕龙办公楼"，//当前地址

        autosend: "yes", //此协议是 自动由定时器触发的
                        //还是手动通过按钮点击触发的
                        //yes为自动触发，no为手动按钮触发

        //如果driver="no",并且 status="waiting",
        //则添加以下目标地址字段
        dst_longitude:"123.123123123",
        dst_latitude: "123.123123123",
        dst_address:"北京市西二旗软件园9号楼"
    }
 *
 * */
    Json json;

    json.insert("sessionid", Data::getInstance()->getSessionid().c_str());
    json.insert("driver", "yes");
    json.insert("status", Data::getInstance()->getStatus());
    json.insert("longitude", longitude);
    json.insert("latitude", latitude);
    json.insert("address", address);
    json.insert("autosend", autoSend);


    string json_str = json.print();


    //释放字符串资源
    env->ReleaseStringUTFChars(jlongitude, longitude);
    env->ReleaseStringUTFChars(jlatitude, latitude);
    env->ReleaseStringUTFChars(jautoSend, autoSend);
    env->ReleaseStringUTFChars(jaddress, address);


    string url = OBO_SERVER_IP;
    url +=":";
    url +=OBO_SERVER_PORT;
    url +="/locationChanged";

    Curl curl(url, true);


    if (curl.execute(json_str) == false) {
        JNIINFO("%s", "curl execute error")
        return JNI_FALSE;
    }


 /*
  *
  *
   ====得到服务器响应数据 ====
     //成功
     {
            result: "ok",
            recode: "0",

            status: "ready",//司机or乘客当前的状态
            orderid: "orderid-xxxx-xxx-xxx-xxxx",//没有则为“NONE”

            //乘客当前坐标，供catching和driving的司机使用，司机端会返回该两个字段
            ptemp_longitude: "13.11112211",
            ptemp_latitude:  "14.11223312"

     }
     //失败
     {
            result: "error",
            recode: "1", //1 -代表session失效，需要重新登录
            reason: "why...."
     }
  *
  *
  * */
    string response_data = curl.responseData();
    Json json_response;
    json_response.parse(curl.responseData());

    string result = json_response.value("result");
    if (result.length() != 0) {
        if (result == "ok") {

            string status = json_response.value("status");
            Data::getInstance()->setStatus(status);

            string orderid = json_response.value("orderid");
            Data::getInstance()->setOrderid(orderid);

            if ((Data::getInstance()->getStatus() == OBO_STATUS_DRIVER_CATCH)
            ||  (Data::getInstance()->getStatus() == OBO_STATUS_DRIVER_DRIVE) ){
                //得到乘客的当前坐标
                Data::getInstance()->set_ptemp_location(json_response.value("ptemp_longitude"),
                                                        json_response.value("ptemp_latitude"));

            }
            else if ((Data::getInstance()->getStatus() == OBO_STATUS_DRIVER_READY)
                    && orderid != "NONE") {
                //已经有乘客下单 需要提示前端司机是否接单
                Data::getInstance()->setStatus(OBO_STATUS_DRIVER_READY);
            }
            else if ((Data::getInstance()->getStatus() == OBO_STATUS_DRIVER_DRIVE)
                    && orderid == "NONE") {
                //说明订单已经结束，完成订单
                Data::getInstance()->setStatus(OBO_STATUS_DRIVER_IDLE);
            }

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