/**
 * @file remote_store.c
 * @brief  全部远程存储请求封装业务
 * @author liu_danbing <liudanbing@chanct.com>
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
 * @brief  远程存储注册信息表 持久
 *
 * @param request_data_buf
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_dataserver_reg(char* request_data_buf)
{
    //unpack json
    int ret = 0;

    cJSON* root = cJSON_Parse(request_data_buf);
    cJSON* username = cJSON_GetObjectItem(root, "username");
    cJSON* password = cJSON_GetObjectItem(root, "password");
    cJSON* isDriver = cJSON_GetObjectItem(root, "driver");
    cJSON* tel      = cJSON_GetObjectItem(root, "tel");
    cJSON* email    = cJSON_GetObjectItem(root, "email");
    cJSON* id_card  = cJSON_GetObjectItem(root, "id_card");

    printf("username = %s\n", username->valuestring);
    printf("password = %s\n", password->valuestring);
    printf("driver   = %s\n", isDriver->valuestring);
    printf("tel      = %s\n", tel->valuestring);
    printf("email    = %s\n", email->valuestring);
    printf("id_card  = %s\n", id_card->valuestring);



    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "insert");
    cJSON_AddStringToObject(request_json, "table", "OBO_TABLE_USER");
    cJSON_AddStringToObject(request_json, "username", username->valuestring);
    cJSON_AddStringToObject(request_json, "password", password->valuestring);
    cJSON_AddStringToObject(request_json, "tel", tel->valuestring);
    cJSON_AddStringToObject(request_json, "email", email->valuestring);
    cJSON_AddStringToObject(request_json, "driver", isDriver->valuestring);
    cJSON_AddStringToObject(request_json, "id_card", id_card->valuestring);

    cJSON_Delete(root);


    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_PER,request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        printf("curl send error\n");
        goto END;
    }


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
    free(request_json_str);
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  远程存储SESSIONID - id_card  临时
 *
 * @param request_data_buf
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_session(char *request_data_buf, char* sessionid)
{
    //unpack json
    int ret = 0;

    cJSON* root = cJSON_Parse(request_data_buf);
    cJSON* id_card  = cJSON_GetObjectItem(root, "id_card");

    printf("sessionid = %s, id_card  = %s\n", sessionid, id_card->valuestring);



    cJSON* request_json = cJSON_CreateObject();
    cJSON_AddStringToObject(request_json, "cmd", "setString");
    cJSON_AddStringToObject(request_json, "unlimited", "no");
    cJSON_AddStringToObject(request_json, "key", sessionid);
    cJSON_AddStringToObject(request_json, "value", id_card->valuestring);
    cJSON_AddNumberToObject(request_json, "lifecycle", 120);

    cJSON_Delete(root);

    char * request_json_str = cJSON_Print(request_json);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));


    if (curl_send(URI_DATA_SERVER_CHE, request_json_str,  &res_data, 1) != 0) {
        ret = -1;
        goto END;
    }


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
    free(request_json_str);
    return ret;
}
