/**
 * @file remote_store.c
 * @brief  全部远程存储请求封装业务
 * @author liu_danbing <danbing_at@163.com>
 * @version 1.0
 * @date 2016-11-25
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <cJSON.h>
#include <curl/curl.h>
#include "busi_cb.h"
#include "util.h"

static size_t reg_curl_to_dataserver_cb(char* ptr, size_t n, size_t m, void* userdata) 
{
    curl_response_data_t *data = (curl_response_data_t*)userdata;

    int response_data_len = n*m;

    memcpy(data->data+data->data_len, ptr, response_data_len);
    data->data_len+=response_data_len;

    return response_data_len;

}


static int curl_send(const char *uri, const char* request_json_str, curl_response_data_t *res_data, int ignoreCA)
{

    //curl --> data server
    CURL *curl = curl_easy_init();

    if (ignoreCA == 1) {
        //忽略https CA认证
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    }

    //设置post数据
    curl_easy_setopt(curl, CURLOPT_URL, uri);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_json_str);


    //设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, reg_curl_to_dataserver_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, res_data);

    //提交请求给数据服务器
    CURLcode code = curl_easy_perform(curl);
    if (code != CURLE_OK) {
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "curl easy perform error code = %d\n", code);
        return -1;
    }

    res_data->data[res_data->data_len] = '\0';

    curl_easy_cleanup(curl);

    return 0;
}

/* -------------------------------------------*/
/**
 * @brief  远程注册用户请求业务
 *
 * @param username
 * @param password
 * @param tel
 * @param email
 * @param isDriver    "yes" or "no"
 * @param id_card       
 *
 * @returns   
 *              0 succ, -1 fail
 */
/* -------------------------------------------*/
int curl_to_dataserver_reg(const char* username, 
        const char* password, 
        const char* tel, 
        const char* email, 
        const char* isDriver, 
        const char* id_card)
{
    int ret = 0;


    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "insert");
    cJSON_AddStringToObject(request_json, "table", "OBO_TABLE_USER");
    cJSON_AddStringToObject(request_json, "busi", "reg");
    cJSON_AddStringToObject(request_json, "username", username);
    //TODO 将password进行md5加密
    cJSON_AddStringToObject(request_json, "password", password);
    cJSON_AddStringToObject(request_json, "tel", tel);
    cJSON_AddStringToObject(request_json, "email", email);
    cJSON_AddStringToObject(request_json, "driver", isDriver);
    cJSON_AddStringToObject(request_json, "id_card", id_card);



    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_PER,request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "curl send error\n");
        goto END;
    }
    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "store reg succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "reg store error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "reg store  error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;
    }
    cJSON_Delete(res_root);





END:
    return ret;
}



/* -------------------------------------------*/
/**
 * @brief  远程查询登陆信息表
 *
 * @param username
 * @param password
 *
 * @returns   
 *  0 succ  -1 fail
 */
/* -------------------------------------------*/
int curl_to_dataserver_login(const char *username,
        const char *password,
        const char *isDriver)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "query");
    cJSON_AddStringToObject(request_json, "busi", "login");
    cJSON_AddStringToObject(request_json, "table", "OBO_TABLE_USER");
    cJSON_AddStringToObject(request_json, "username", username);
    //TODO 将password进行md5加密
    cJSON_AddStringToObject(request_json, "password", password);
    cJSON_AddStringToObject(request_json, "driver", isDriver);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));

    if (curl_send(URI_DATA_SERVER_PER, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "curl_to_dataserver_login send error, %s, %s, %s\n", username, password, isDriver);
        goto END;
    }
    cJSON_Delete(request_json);
    free(request_json_str);

    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "query login succ\n");
        ret = 0;
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "query login, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL,"query  login, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);



END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  创建订单 持久型
 *
 * @param order
 *
 * @returns   
 *    0 succ    -1 fail
 */
/* -------------------------------------------*/
int curl_to_dataserver_create_order(order_t *order)
{
    int ret = 0;


    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "insert");
    cJSON_AddStringToObject(request_json, "table", "OBO_TABLE_ORDER");
    cJSON_AddStringToObject(request_json, "busi", "create_order");


    cJSON_AddStringToObject(request_json, "orderid", order->orderid);
    cJSON_AddStringToObject(request_json, "passenger_username", order->passenger_username);
    cJSON_AddStringToObject(request_json, "driver_username", order->driver_username);

    cJSON_AddStringToObject(request_json, "create_order_time", order->create_order_time);
    cJSON_AddStringToObject(request_json, "start_order_time", order->start_order_time);
    cJSON_AddStringToObject(request_json, "end_time", order->end_time);

    cJSON_AddStringToObject(request_json, "src_address", order->src_address);
    cJSON_AddStringToObject(request_json, "dst_address", order->dst_address);
    cJSON_AddStringToObject(request_json, "src_longitude", order->src_longitude);
    cJSON_AddStringToObject(request_json, "src_latitude", order->src_latitude);
    cJSON_AddStringToObject(request_json, "dst_longitude", order->dst_longitude);
    cJSON_AddStringToObject(request_json, "dst_latitude", order->dst_latitude);

    cJSON_AddStringToObject(request_json, "src_address_real", order->src_address_real);
    cJSON_AddStringToObject(request_json, "dst_address_real", order->dst_address_real);
    cJSON_AddStringToObject(request_json, "src_longitude_real", order->src_longitude_real);
    cJSON_AddStringToObject(request_json, "src_latitude_real", order->src_latitude_real);
    cJSON_AddStringToObject(request_json, "dst_longitude_real", order->dst_longitude_real);
    cJSON_AddStringToObject(request_json, "dst_latitude_real", order->dst_latitude_real);

    cJSON_AddStringToObject(request_json, "RMB", order->RMB);

    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_PER,request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "curl send error\n");
        goto END;
    }
    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "create order succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "create order error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "create order  error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;
    }
    cJSON_Delete(res_root);

END:
    return ret;

}


/* -------------------------------------------*/
/**
 * @brief  获取当前坐标周边最近司机信息
 *
 * @param key
 * @param longitude
 * @param latitude
 * @param radius
 * @param geo_drivers_p  OUT 得到的所有司机数据
 * @param driver_count  OUT 得到的所有司机的数量
 *
 * @returns   
 *      0 succ ,  -1 fail
 */
/* -------------------------------------------*/
int curl_to_cacheserver_radiusGeo(const char *key, const char *longitude, const char *latitude,const char *radius, geo_drvier_t **geo_drivers_p, int *driver_count)
{
    int ret = 0;
    int i = 0;
    /*
       ====给服务端的协议====   
    https://ip:port/cache [json_data]  
    {
        cmd:      "radiusGeo",
        key:      "ONLINE_DRIVER_GEO_ZSET",
        longitude:  "98.123123123",
        latitude:   "39.123123123",
        radius:    "200"  //方圆多少米
    }
    */
    cJSON *request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "radiusGeo");
    cJSON_AddStringToObject(request_json, "key", KEY_ONLINE_DRIVERS_GEO_ZSET);
    cJSON_AddStringToObject(request_json, "longitude", longitude);
    cJSON_AddStringToObject(request_json, "latitude", latitude);
    cJSON_AddStringToObject(request_json, "radius", radius);

    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    /*
    ====得到服务器响应数据====
    //成功
    {
        result: "ok",
        count:  2, //附近司机个数
        drivers: 
        [
            {
                sessionid:"online-driver-xxxx-xxx-xxx-xxx-xxxx",
                distance: "10",//与自己距离多少米
                longitude:  "97.123123123",
                latitude:   "39.123123123"
            }，
            {
                sessionid:"online-driver-xxxx-xxx-xxx-xxx-xxxx",
                distance: "15",//与自己距离多少米
                longitude:  "99.123123123",
                latitude:   "39.123123123"
            }
        ]
    }
    //失败
    {
        result: "error",
        recode: "2", //1 代表此key不存在
                     //2 操作失败
        reason: "why...."
    }
    */
    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get radius geo  succ\n");

        cJSON *count = cJSON_GetObjectItem(res_root, "count");
        if (count && (count->valueint >= 0)) {

            geo_drvier_t *geo_drivers  = malloc(sizeof(geo_drvier_t) * count->valueint);
            cJSON *drivers = cJSON_GetObjectItem(res_root, "drivers");

            for (i = 0; i < count->valueint; i++) {
                cJSON *obj = cJSON_GetArrayItem(drivers, i);
                cJSON *sessionid = cJSON_GetObjectItem(obj, "sessionid");
                cJSON *distance = cJSON_GetObjectItem(obj, "distance");
                cJSON *longitude = cJSON_GetObjectItem(obj, "longitude");
                cJSON *latitude = cJSON_GetObjectItem(obj, "latitude");
            
                strncpy(geo_drivers[i].sessionid, sessionid->valuestring, SESSIONID_STR_LEN);
                strncpy(geo_drivers[i].distance, distance->valuestring, DISTANCE_STR_LEN);
                strncpy(geo_drivers[i].longitude, longitude->valuestring, LOCATION_POINT_STR_LEN);
                strncpy(geo_drivers[i].latitude, latitude->valuestring, LOCATION_POINT_STR_LEN);
            }

            *geo_drivers_p = geo_drivers;
            *driver_count = count->valueint;
        }

    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  判读sessionid是否存在
 *
 * @param sessionid
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_existsessionid(const char *sessionid)
{
    int ret = 0;
    /*
    ====给服务端的协议====   
        https://ip:port/cache [json_data]  
        {
             cmd: "existKey",
             key: "online-driver-xxxxx-xxx-xxx-xxx-xxxx"
        }

     ====得到服务器响应数据====
       //成功
       {
            result: "ok",//存在
            recode: "0",
      }
      //失败
      {
           result: "error",
           recode: "1", //1 代表此key不存在
                        //2 操作失败
           reason: "why...."
      }

     */

    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "existKey");
    cJSON_AddStringToObject(request_json, "key", sessionid);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));

    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "curl send error\n");
        goto END;
    }
    cJSON_Delete(request_json);
    free(request_json_str);



    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "exist key   succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "exist  key error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "exist key error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief   远程设置一个key的超时时间(生命周期)
 *
 * @param sessionid
 * @param lifecycle
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_lifecycle(const char *sessionid, int lifecycle)
{
    int ret = 0;
    /*
       {
cmd:     "setLifcycle",
key:     "online-driver-[sessionid]",
lifecycle: 600
}

====得到服务器响应数据====
    //成功
    {
result: "ok",
recode: "0"
}
    //失败
    {
result: "error",
recode: "2", //1 代表此key不存在
    //2 操作失败
reason: "why...."
}
     */

    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "setLifecycle");
    cJSON_AddStringToObject(request_json, "key", sessionid);
    cJSON_AddNumberToObject(request_json, "lifecycle", lifecycle);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));

    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "curl send error\n");
        goto END;
    }
    cJSON_Delete(request_json);
    free(request_json_str);



    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set lifecycle  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set lifecycle error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set lifecycle error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}


/* -------------------------------------------*/
/**
 * @brief  远程存储SESSIONID  临时
 *
 * @param username
 * @param sessionid 
 * @param orderid 
 *
 * @returns   
 *          0 succ, -1 fail
 */
/* -------------------------------------------*/
int curl_to_cacheserver_session(const char *username,  const char* sessionid, const char *orderid)
{
    int ret = 0;

    LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "sessionid = %s, username  = %s\n", sessionid, username);

    /*
       ====给服务端的协议====   
https://ip:port/cache [json_data]  
{
cmd:     "setHash",
key:     "online-driver-[sessionid]",
fields:   ["username","orderid"],
values:   ["盖伦", "NONE"]
}

====得到服务器响应数据====
    //成功
    {
result: "ok",
recode: "0"
}
    //失败
    {
result: "error",
recode: "2", //1 代表此key不存在
    //2 操作失败
reason: "why...."
}
     */

    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "setHash");
    cJSON_AddStringToObject(request_json, "key", sessionid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("username"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("orderid"));

    cJSON* values = cJSON_CreateArray();
    cJSON_AddItemToArray(values, cJSON_CreateString(username));
    cJSON_AddItemToArray(values, cJSON_CreateString(orderid));

    cJSON_AddItemToObject(request_json, "fields", fields);
    cJSON_AddItemToObject(request_json, "values", values);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:
    return ret;
}

/*
        创建临时订单 
 */
int curl_to_cacheserver_create_order(const char *orderid,
                                     const char *passenger_sessionid,
                                     const char *driver_sessionid,
                                     const char *create_order_time,
                                     const char *src_address,
                                     const char *dst_address,
                                     const char *src_longitude,
                                     const char *src_latitude,
                                     const char *dst_longitude,
                                     const char *dst_latitude,
                                     const char *RMB)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "setHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("order_status"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("passenger_sessionid"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("driver_sessionid"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("create_order_time"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_address"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_address"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_latitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_latitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("RMB"));

    cJSON* values = cJSON_CreateArray();
    cJSON_AddItemToArray(values, cJSON_CreateString("INIT"));
    cJSON_AddItemToArray(values, cJSON_CreateString(passenger_sessionid));
    cJSON_AddItemToArray(values, cJSON_CreateString(driver_sessionid));
    cJSON_AddItemToArray(values, cJSON_CreateString(create_order_time));
    cJSON_AddItemToArray(values, cJSON_CreateString(src_address));
    cJSON_AddItemToArray(values, cJSON_CreateString(dst_address));
    cJSON_AddItemToArray(values, cJSON_CreateString(src_longitude));
    cJSON_AddItemToArray(values, cJSON_CreateString(src_latitude));
    cJSON_AddItemToArray(values, cJSON_CreateString(dst_longitude));
    cJSON_AddItemToArray(values, cJSON_CreateString(dst_latitude));
    cJSON_AddItemToArray(values, cJSON_CreateString(RMB));

    cJSON_AddItemToObject(request_json, "fields", fields);
    cJSON_AddItemToObject(request_json, "values", values);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "create order error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "create order error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  设置订单首次启动 相关数据
 *
 * @param orderid
 * @param order_status
 * @param start_driving_time
 * @param src_address_real
 * @param src_longitude_real
 * @param src_latitude_real
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_startorder_real(char *orderid, char *order_status, char *start_driving_time,char *src_address_real, char *src_longitude_real, char *src_latitude_real)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "setHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("order_status"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("start_driving_time"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_address_real"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_longitude_real"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_latitude_real"));

    cJSON* values = cJSON_CreateArray();
    cJSON_AddItemToArray(values, cJSON_CreateString(order_status));
    cJSON_AddItemToArray(values, cJSON_CreateString(start_driving_time));
    cJSON_AddItemToArray(values, cJSON_CreateString(src_address_real));
    cJSON_AddItemToArray(values, cJSON_CreateString(src_longitude_real));
    cJSON_AddItemToArray(values, cJSON_CreateString(src_latitude_real));

    cJSON_AddItemToObject(request_json, "fields", fields);
    cJSON_AddItemToObject(request_json, "values", values);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "start order real  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "start  order real error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "start order real, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  根据sessionid得到对应的orderid
 *
 * @param sessionid
 * @param order_id   OUT 得到的orderid
 *
 * @returns   
 *          0 succ, -1 fail
 */
/* -------------------------------------------*/
int curl_to_cacheserver_get_orderid (const char *sessionid, char *orderid) 
{
    int ret = 0;

    /*
       ====给服务端的协议====   
    https://ip:port/cache [json_data]  
    {
        cmd:      "getHash",
        key:      "online_driver-[sessionid]",
        fields:    ["orderid"]  
    }
    */

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "getHash");
    cJSON_AddStringToObject(request_json, "key", sessionid);
    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("orderid"));
    cJSON_AddItemToObject(request_json, "fields", fields);

    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    /*
    ====得到服务器响应数据====
    //成功
    {
        result: "ok",
        count:  1, 
        values: ["order-id-xxx-xx-xxx-xx"]
    }
    //失败
    {
        result: "error",
        recode: "2", //1 代表此key不存在
                     //2 操作失败
        reason: "why...."
    }
    */
    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get orderid   succ\n");

        cJSON *count = cJSON_GetObjectItem(res_root, "count");
        if (count && (count->valueint >= 0)) {

            cJSON *values = cJSON_GetObjectItem(res_root, "values");
            int i = 0;

            for (i = 0; i < count->valueint; i++) {
                if (i == 0) {
                    cJSON *orderid_obj = cJSON_GetArrayItem(values, i);
                    strncpy(orderid, orderid_obj->valuestring, ORDERID_STR_LEN);
                }
            }
        }

    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  将sessionid 从 坐标池中删除
 *
 * @param sessionid
 *
 * @returns   
 *      0 succ, -1 fail
 */
/* -------------------------------------------*/
int curl_to_cacheserver_remGeo(const char *sessionid)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "remZset");
    cJSON_AddStringToObject(request_json, "key", KEY_ONLINE_DRIVERS_GEO_ZSET);
    cJSON_AddStringToObject(request_json, "member", sessionid);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "remZset succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "remZset error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "remZset error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);
END:
    return ret;
}


/* -------------------------------------------*/
/**
 * @brief   向 坐标池 中添加一条数据
 *
 * @param sessionid
 * @param longitude
 * @param latitude
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_addGeo(const char *sessionid, const char *longitude, const char *latitude) 
{

    /*
       ====给服务端的协议====   
           https://ip:port/cache [json_data]  
        {
          cmd:     "addGeo",
          key:     "ONLINE_DRIVER_GEO_ZSET",
          member:   "online-driver-xxxxx-xxx-xxx-xxx-xxxx",
          longitude:   "98.123123123",
          latitude:   "39.123123123"
        }
        ====得到服务器响应数据====
        //成功
        {
            result: "ok",
            recode: "0"
        }
        //失败
        {    
         result: "error",
         recode: "2", //1 代表此key不存在
                    //2 操作失败
         reason: "why...."
        }
     */
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "addGeo");
    cJSON_AddStringToObject(request_json, "key", KEY_ONLINE_DRIVERS_GEO_ZSET);
    cJSON_AddStringToObject(request_json, "member", sessionid);
    cJSON_AddStringToObject(request_json, "longitude", longitude);
    cJSON_AddStringToObject(request_json, "latitude", latitude);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "add Geo succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "add Geo error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "add Geo error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);
END:
    return ret;

    

    return ret;
}

/* -------------------------------------------*/
/**
 * @brief   设置当前司机的临时坐标地址信息
 *
 * @param orderid
 * @param dtemp_longitude
 * @param dtemp_latitude
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_set_dtemp_location(const char *orderid, const char *dtemp_longitude, const char *dtemp_latitude) 
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "setHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("dtemp_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dtemp_latitude"));

    cJSON* values = cJSON_CreateArray();
    cJSON_AddItemToArray(values, cJSON_CreateString(dtemp_longitude));
    cJSON_AddItemToArray(values, cJSON_CreateString(dtemp_latitude));

    cJSON_AddItemToObject(request_json, "fields", fields);
    cJSON_AddItemToObject(request_json, "values", values);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set dtemp location  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set dtemp location error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set dtemp location , unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:

    return ret;
}


/* -------------------------------------------*/
/**
 * @brief  设置当前乘客的临时坐标地址信息
 *
 * @param orderid
 * @param ptemp_longitude
 * @param ptemp_latitude
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_set_ptemp_location(const char *orderid, const char *ptemp_longitude, const char *ptemp_latitude) 
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "setHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("ptemp_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("ptemp_latitude"));

    cJSON* values = cJSON_CreateArray();
    cJSON_AddItemToArray(values, cJSON_CreateString(ptemp_longitude));
    cJSON_AddItemToArray(values, cJSON_CreateString(ptemp_latitude));

    cJSON_AddItemToObject(request_json, "fields", fields);
    cJSON_AddItemToObject(request_json, "values", values);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set ptemp location  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set ptemp location error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set ptemp location , unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:

    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  获取当前司机的临时坐标地址信息
 *
 * @param orderid
 * @param dtemp_longitude   OUT
 * @param dtemp_latitude    OUT
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_get_dtemp_location(const char *orderid, char *dtemp_longitude, char *dtemp_latitude)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "getHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("dtemp_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dtemp_latitude"));

    cJSON_AddItemToObject(request_json, "fields", fields);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get dtemp location succ\n");

        cJSON *count = cJSON_GetObjectItem(res_root, "count");
        if (count && (count->valueint >= 0)) {

            cJSON *values = cJSON_GetObjectItem(res_root, "values");
            int i = 0;

            for (i = 0; i < count->valueint; i++) {
                if (i == 0) {
                    //dtemp_longitude
                    cJSON *longitude = cJSON_GetArrayItem(values, i);
                    strncpy(dtemp_longitude, longitude->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 1) {
                    //dtemp_latitude
                    cJSON *latitude = cJSON_GetArrayItem(values, i);
                    strncpy(dtemp_latitude, latitude->valuestring, LOCATION_POINT_STR_LEN);
                }
            }
        }

    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get dtemp location error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get dtemp location error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  获取当前乘客的临时坐标地址信息
 *
 * @param orderid
 * @param ptemp_longitude   OUT
 * @param ptemp_latitude    OUT
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_get_ptemp_location(const char *orderid, char *ptemp_longitude, char *ptemp_latitude)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "getHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("ptemp_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("ptemp_latitude"));

    cJSON_AddItemToObject(request_json, "fields", fields);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get ptemp location succ\n");

        cJSON *count = cJSON_GetObjectItem(res_root, "count");
        if (count && (count->valueint >= 0)) {

            cJSON *values = cJSON_GetObjectItem(res_root, "values");
            int i = 0;

            for (i = 0; i < count->valueint; i++) {
                if (i == 0) {
                    //ptemp_longitude
                    cJSON *longitude = cJSON_GetArrayItem(values, i);
                    strncpy(ptemp_longitude, longitude->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 1) {
                    //ptemp_latitude
                    cJSON *latitude = cJSON_GetArrayItem(values, i);
                    strncpy(ptemp_latitude, latitude->valuestring, LOCATION_POINT_STR_LEN);
                }
            }
        }

    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get ptemp location error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get ptemp location error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  获取当前临时订单中的 订单状态
 *
 * @param orderid
 * @param order_status   OUT
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_get_order_status(const char *orderid, char *order_status)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "getHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("order_status"));

    cJSON_AddItemToObject(request_json, "fields", fields);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get dtemp location succ\n");

        cJSON *count = cJSON_GetObjectItem(res_root, "count");
        if (count && (count->valueint >= 0)) {

            cJSON *values = cJSON_GetObjectItem(res_root, "values");
            int i = 0;

            for (i = 0; i < count->valueint; i++) {
                if (i == 0) {
                    //order_status
                    cJSON *order_status_obj = cJSON_GetArrayItem(values, i);
                    strncpy(order_status, order_status_obj->valuestring, ORDERID_STR_LEN);
                }
            }
        }

    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get dtemp location error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get dtemp location error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}


/* -------------------------------------------*/
/**
 * @brief  删除一个缓存表
 *
 * @param key
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_delete_key(const char *key)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "deleteKey");
    cJSON_AddStringToObject(request_json, "key", key);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "delete key  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "delete key error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "delete key error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:

    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  设置当前用户拥有订单号
 *
 * @param sessionid
 * @param orderid
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_set_orderid (const char *sessionid,
                                     const char *orderid)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "setHash");
    cJSON_AddStringToObject(request_json, "key", sessionid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("orderid"));

    cJSON* values = cJSON_CreateArray();
    cJSON_AddItemToArray(values, cJSON_CreateString(orderid));

    cJSON_AddItemToObject(request_json, "fields", fields);
    cJSON_AddItemToObject(request_json, "values", values);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set session  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set order error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "set order error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  通过sesionid 得到对应的用户名
 *
 * @param sessionid
 * @param username      OUT
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_get_username(const char *sessionid, char *username)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "getHash");
    cJSON_AddStringToObject(request_json, "key", sessionid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("username"));

    cJSON_AddItemToObject(request_json, "fields", fields);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get username succ\n");

        cJSON *count = cJSON_GetObjectItem(res_root, "count");
        if (count && (count->valueint >= 0)) {

            cJSON *values = cJSON_GetObjectItem(res_root, "values");
            int i = 0;

            for (i = 0; i < count->valueint; i++) {
                if (i == 0) {
                    //username
                    cJSON *username_obj = cJSON_GetArrayItem(values, i);
                    strncpy(username, username_obj->valuestring, USERNAME_STR_LEN);
                }
            }
        }

    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get username error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get username error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  通过临时订单得到 订单信息
 *
 * @param orderid
 * @param passenger_sessionid OUT 临时订单的乘客sessionid
 * @param driver_sessionid  OUT 临时订单的司机sessionid
 * @param order_info
 *
 * @returns   
 *          0 succ, -1 fail
 */
/* -------------------------------------------*/
int curl_to_cacheserver_get_orderd ( const char *orderid, char*passenger_sessionid, char *driver_sessionid, order_t *order_info)
{
    int ret = 0;

    cJSON* request_json = cJSON_CreateObject();

    cJSON_AddStringToObject(request_json, "cmd", "getHash");
    cJSON_AddStringToObject(request_json, "key", orderid);

    cJSON* fields = cJSON_CreateArray();
    cJSON_AddItemToArray(fields, cJSON_CreateString("passenger_sessionid"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("driver_sessionid"));

    cJSON_AddItemToArray(fields, cJSON_CreateString("create_order_time"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("start_order_time"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("end_time"));
    
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_address"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_address"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_latitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_longitude"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_latitude"));

    cJSON_AddItemToArray(fields, cJSON_CreateString("src_address_real"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_address_real"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_longitude_real"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("src_latitude_real"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_longitude_real"));
    cJSON_AddItemToArray(fields, cJSON_CreateString("dst_latitude_real"));

    cJSON_AddItemToArray(fields, cJSON_CreateString("RMB"));

    cJSON_AddItemToObject(request_json, "fields", fields);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }

    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get order info  succ\n");

        //orderid
        strncpy(order_info->orderid, orderid, ORDERID_STR_LEN);


        cJSON *count = cJSON_GetObjectItem(res_root, "count");
        if (count && (count->valueint >= 0)) {

            cJSON *values = cJSON_GetObjectItem(res_root, "values");
            int i = 0;

            for (i = 0; i < count->valueint; i++) {
                if (i == 0) {
                    //passenger_sessionid
                    cJSON *p_sessionid_obj = cJSON_GetArrayItem(values, i);
                    strncpy(passenger_sessionid, p_sessionid_obj->valuestring, SESSIONID_STR_LEN);
                }
                else if (i == 1) {
                    //driver_sessionid
                    cJSON *d_sessionid_obj = cJSON_GetArrayItem(values, i);
                    strncpy(driver_sessionid, d_sessionid_obj->valuestring, SESSIONID_STR_LEN);
                }
                else if (i == 2) {
                    //create_order_time
                    cJSON *create_order_time = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->create_order_time, create_order_time->valuestring, TIME_STR_LEN);
                }
                else if (i == 3) {
                    //start_order_time
                    cJSON *start_order_time = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->start_order_time, start_order_time->valuestring, TIME_STR_LEN);
                }
                else if (i == 4) {
                    //end_time
                    cJSON *end_time = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->end_time, end_time->valuestring, TIME_STR_LEN);
                }
                else if (i == 5) {
                    //src_address
                    cJSON *src_address = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->src_address, src_address->valuestring, ADDRESS_STR_LEN);
                }
                else if (i == 6) {
                    //dst_address
                    cJSON *dst_address = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->dst_address, dst_address->valuestring, ADDRESS_STR_LEN);
                }
                else if (i == 7) {
                    //src_longitude
                    cJSON *src_longitude = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->src_longitude, src_longitude->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 8) {
                    //src_latitude
                    cJSON *src_latitude = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->src_latitude, src_latitude->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 9) {
                    //dst_longitude
                    cJSON *dst_longitude = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->dst_longitude, dst_longitude->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 10) {
                    //dst_latitude
                    cJSON *dst_latitude = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->dst_latitude, dst_latitude->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 11) {
                    //src_address_real
                    cJSON *src_address_real = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->src_address_real, src_address_real->valuestring, ADDRESS_STR_LEN);
                }
                else if (i == 12) {
                    //dst_address_real
                    cJSON *dst_address_real = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->dst_address_real, dst_address_real->valuestring, ADDRESS_STR_LEN);
                }
                else if (i == 13) {
                    //src_longitude_real
                    cJSON *src_longitude_real = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->src_longitude_real, src_longitude_real->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 14) {
                    //src_latitude_real
                    cJSON *src_latitude_real = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->src_latitude_real, src_latitude_real->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 15) {
                    //dst_longitude_real
                    cJSON *dst_longitude_real = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->dst_longitude_real, dst_longitude_real->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 16) {
                    //dst_latitude_real
                    cJSON *dst_latitude_real = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->dst_latitude_real, dst_latitude_real->valuestring, LOCATION_POINT_STR_LEN);
                }
                else if (i == 17) {
                    //RMB
                    cJSON *RMB = cJSON_GetArrayItem(values, i);
                    strncpy(order_info->RMB, RMB->valuestring, 64);
                }
            }
        }

    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get order info  error, reason = %s\n", reason->valuestring);
        }
        else {
            LOG(LOG_MODULE, LOG_PROC_REMOTE_CURL, "get order info error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);

END:
    return ret;
}
