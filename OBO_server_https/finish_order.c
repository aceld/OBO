/**
 * @file finish_order.c
 * @brief  完成订单模块
 * @author liu_danbing <danbing_at@163.com>
 * @version 1.0
 * @date 2016-12-06
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

#include <curl/curl.h>
#include "cJSON.h"
#include "https-common.h"
#include "busi_cb.h"
#include "util.h"


void finish_order_cb (struct evhttp_request *req, void *arg)
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
        LOG(LOG_MODULE, LOG_PROC_FINISHORDER, "get uri:%s\n", uri);
        return;
    }

    /* 这里只处理Post请求, Get请求，就直接return 200 OK  */
    if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
    { 
        evhttp_send_reply (req, 200, "OK", NULL);
        return;
    }

    printf ("Got a POST request for <%s>\n", uri);
    LOG(LOG_MODULE, LOG_PROC_FINISHORDER, "Got a POST request for <%s>\n", uri);

    //判断此URI是否合法
    decoded = evhttp_uri_parse (uri);
    if (! decoded)
    { 
        printf ("It's not a good URI. Sending BADREQUEST\n");
        LOG(LOG_MODULE, LOG_PROC_FINISHORDER, "It's not a good URI. Sending BADREQUEST\n");
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
    printf("[post_data][%d]=\n%s\n", post_data_len, payload);
    LOG(LOG_MODULE, LOG_PROC_FINISHORDER, "[post_data][%d]=\n%s\n", post_data_len, payload);


    /*
       具体的：可以根据Post的参数执行相应操作，然后将结果输出
       ...
     */
    //=======================================================
    //unpack json
    cJSON* root = cJSON_Parse(request_data_buf);
    cJSON* sessionid = cJSON_GetObjectItem(root, "sessionid");
    cJSON* orderid = cJSON_GetObjectItem(root, "orderid");
    

    printf("sessionid = %s\n", sessionid->valuestring);
    printf("orderid = %s\n", orderid->valuestring);

    char* recode = RECODE_OK;
    order_t *order_info = malloc(ORDER_INFO_LEN);
    memset(order_info, 0, ORDER_INFO_LEN);

    char passenger_sessionid[SESSIONID_STR_LEN] = {0};
    char driver_sessionid[SESSIONID_STR_LEN] = {0};

    //得到订单信息
    ret = curl_to_cacheserver_get_orderd(orderid->valuestring, passenger_sessionid, driver_sessionid, order_info);

    if (ret == 0) {
        //根据passenger_sessionid 得到乘客用户名      
        ret = curl_to_cacheserver_get_username(passenger_sessionid, order_info->passenger_username);
    }

    if (ret == 0) {
        //根据driver_sessionid 得到司机用户名      
        ret = curl_to_cacheserver_get_username(driver_sessionid, order_info->driver_username);
    }

    if (ret == 0) {
        //创建 持久 订单
        ret = curl_to_dataserver_create_order(order_info);
    }

    if (ret == 0) {
        //将 司机的 sessionID中的 orderid 设置为 NONE
        ret = curl_to_cacheserver_set_orderid(driver_sessionid, "NONE");
    }

    if (ret == 0) {
        //将 乘客的 sessionID中的 orderid 设置为 NONE
        ret = curl_to_cacheserver_set_orderid(passenger_sessionid, "NONE");
    }

    if (ret == 0) {
        //删除orderid
        ret = curl_to_cacheserver_delete_key(orderid->valuestring);
    }


    //将sessionid存放到缓存数据库中
    char *response_data = make_gen_res_json(ret, recode, "get finish order ERROR");

    free(order_info);
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
    LOG(LOG_MODULE, LOG_PROC_FINISHORDER, "[response]:\n");
    LOG(LOG_MODULE, LOG_PROC_FINISHORDER, "%s\n", response_data);

    free(response_data);
}
