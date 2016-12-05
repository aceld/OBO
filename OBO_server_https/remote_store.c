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
        printf("curl easy perform error code = %d\n", code);
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
        printf("curl send error\n");
        goto END;
    }
    cJSON_Delete(request_json);
    free(request_json_str);


    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        printf("store reg succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            printf("reg store error, reason = %s\n", reason->valuestring);
        }
        else {
            printf("reg store  error, unknow reason, res_data=%s\n", res_data.data);
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
        printf("curl send error\n");
        goto END;
    }
    cJSON_Delete(request_json);
    free(request_json_str);

    cJSON *res_root = cJSON_Parse(res_data.data);

    cJSON *result = cJSON_GetObjectItem(res_root, "result");
    if (result && (strcmp(result->valuestring, "ok") == 0)) {
        //succ
        printf("query login succ\n");
        ret = 0;
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            printf("query login, reason = %s\n", reason->valuestring);
        }
        else {
            printf("query  login, unknow reason, res_data=%s\n", res_data.data);
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
        count:  "2", //附近司机个数
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
        printf("get radius geo  succ\n");

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
            printf("set session error, reason = %s\n", reason->valuestring);
        }
        else {
            printf("set session error, unknow reason, res_data=%s\n", res_data.data);
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
        printf("curl send error\n");
        goto END;
    }
cJSON_Delete(request_json);
free(request_json_str);



cJSON *res_root = cJSON_Parse(res_data.data);

cJSON *result = cJSON_GetObjectItem(res_root, "result");
if (result && (strcmp(result->valuestring, "ok") == 0)) {
    //succ
    printf("set lifecycle  succ\n");
}
else {
    //fail
    cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
    if (reason) {
        printf("set lifecycle error, reason = %s\n", reason->valuestring);
    }
    else {
        printf("set lifecycle error, unknow reason, res_data=%s\n", res_data.data);
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

    printf("sessionid = %s, username  = %s\n", sessionid, username);

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
        printf("set session  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            printf("set session error, reason = %s\n", reason->valuestring);
        }
        else {
            printf("set session error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:
    return ret;
}

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
        printf("set session  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            printf("create order error, reason = %s\n", reason->valuestring);
        }
        else {
            printf("create order error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:
    return ret;
}


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
        printf("set session  succ\n");
    }
    else {
        //fail
        cJSON *reason = cJSON_GetObjectItem(res_root, "reason");
        if (reason) {
            printf("set order error, reason = %s\n", reason->valuestring);
        }
        else {
            printf("set order error, unknow reason, res_data=%s\n", res_data.data);
        }

        ret = -1;

    }
    cJSON_Delete(res_root);


END:
    return ret;
}
