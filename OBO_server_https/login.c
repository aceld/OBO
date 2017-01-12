/**
 * @file login.c
 * @brief  登陆模块
 * @author liu_danbing <danbing_at@chanct.com>
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






void login_cb (struct evhttp_request *req, void *arg)
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
        LOG(LOG_MODULE, LOG_PROC_LOGIN, "get uri:%s", uri);
        return;
    }

    /* 这里只处理Post请求, Get请求，就直接return 200 OK  */
    if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
    { 
        evhttp_send_reply (req, 200, "OK", NULL);
        return;
    }

    printf ("Got a POST request for <%s>\n", uri);
    LOG(LOG_MODULE, LOG_PROC_LOGIN, "Got a POST request for <%s>", uri);


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
    LOG(LOG_MODULE, LOG_PROC_LOGIN, "[post_data][%d]=\n %s\n", post_data_len, payload);



    /*
       具体的：可以根据Post的参数执行相应操作，然后将结果输出
       ...
     */
    //=======================================================
    //unpack json
    cJSON* root = cJSON_Parse(request_data_buf);
    cJSON* username = cJSON_GetObjectItem(root, "username");
    cJSON* password = cJSON_GetObjectItem(root, "password");
    cJSON* isDriver = cJSON_GetObjectItem(root, "driver");

    printf("username = %s\n", username->valuestring);
    printf("password = %s\n", password->valuestring);
    printf("isDriver = %s\n", isDriver->valuestring);

    ret = curl_to_dataserver_login(username->valuestring, password->valuestring, isDriver->valuestring);

    char *recode = RECODE_OK;

    char sessionid[SESSIONID_STR_LEN] = {0};
    if (ret == 0) {
        //生成sessionid
        create_sessionid(isDriver->valuestring, sessionid);
        ret = curl_to_cacheserver_session(username->valuestring, sessionid, ORDER_ID_NONE);
    }

    if (ret == 0) {
        //设置key的超时时间
        ret = curl_to_cacheserver_lifecycle(sessionid, SESSIONID_LIFECYCLE);
    }


    //将sessionid存放到缓存数据库中
    char *response_data = make_reg_login_res_json(ret, recode, sessionid, "login error");

    cJSON_Delete(root);
    // =========================================================

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
    LOG(LOG_MODULE, LOG_PROC_LOGIN,"[response]:\n");
    LOG(LOG_MODULE, LOG_PROC_LOGIN, "%s\n", response_data);

    free(response_data);
}
