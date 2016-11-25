/**
 * @file reg.c
 * @brief  注册模块
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

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#include <cJSON.h>
#include <curl/curl.h>
#include "https-common.h"
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


static int reg_curl_to_dataserver(char* request_data_buf)
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




    //curl --> data server
    CURL *curl = curl_easy_init();

    //忽略https CA认证
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    //设置post数据
    curl_easy_setopt(curl, CURLOPT_URL, URI_DATA_SERVER_PER);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_json_str);


    //设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, reg_curl_to_dataserver_cb);
    curl_response_data_t res_data;
    memset(&res_data, 0, sizeof(res_data));
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res_data);

    //提交请求给数据服务器
    CURLcode code = curl_easy_perform(curl);
    if (code != CURLE_OK) {
        ret = -1;  
        printf("curl easy perform error code = %d\n", code);
        goto END;
    }

    res_data.data[res_data.data_len] = '\0';

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
    curl_easy_cleanup(curl);
    free(request_json_str);
    return ret;
}



/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
    void
reg_cb (struct evhttp_request *req, void *arg)
{ 
    int ret = 0;
    struct evbuffer *evb = NULL;
    const char *uri = evhttp_request_get_uri (req);
    struct evhttp_uri *decoded = NULL;

    /* 判断 req 是否是GET 请求 */
    if (evhttp_request_get_command (req) == EVHTTP_REQ_GET)
    {
        struct evbuffer *buf = evbuffer_new();
        if (buf == NULL) return;
        evbuffer_add_printf(buf, "Requested: %s\n", uri);
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
        printf("get uri:%s\n", uri);
        return;
    }

    /* 这里只处理Post请求, Get请求，就直接return 200 OK  */
    if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
    { 
        evhttp_send_reply (req, 200, "OK", NULL);
        return;
    }

    printf ("Got a POST request for <%s>\n", uri);

    //判断此URI是否合法
    decoded = evhttp_uri_parse (uri);
    if (! decoded)
    { 
        printf ("It's not a good URI. Sending BADREQUEST\n");
        evhttp_send_error (req, HTTP_BADREQUEST, 0);
        return;
    }

    /* Decode the payload */
    struct evbuffer *buf = evhttp_request_get_input_buffer (req);
    evbuffer_add (buf, "", 1);    /* NUL-terminate the buffer */
    char *payload = (char *) evbuffer_pullup (buf, -1);
    int post_data_len = evbuffer_get_length(buf);
    char request_data_buf[4096] = {0};
    memcpy(request_data_buf, payload, post_data_len);
    printf("[post_data][%d]=\n %s\n", post_data_len, payload);


    /*
       具体的：可以根据Post的参数执行相应操作，然后将结果输出
       ...
     */


    // 发送libcurl请求
    ret = reg_curl_to_dataserver(request_data_buf);

    char *response_data = make_reg_login_res_json(ret, "reg error");




    /* This holds the content we're sending. */

    //HTTP header
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Connection", "close");

    evb = evbuffer_new ();
    evbuffer_add_printf(evb, "%s", response_data);
    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", evb);

    if (decoded)
        evhttp_uri_free (decoded);
    if (evb)
        evbuffer_free (evb);


    printf("[response]:\n");
    printf("%s\n", response_data);

    free(response_data);
}
