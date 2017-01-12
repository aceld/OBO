/**
 * @file set_order.c
 * @brief  下单模块
 * @author liu_danbing <danbing_at@163.com>
 * @version 1.0
 * @date 2016-12-01
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


void set_order_cb (struct evhttp_request *req, void *arg)
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
        LOG(LOG_MODULE, LOG_PROC_SETORDER, "get uri:%s\n", uri);
        return;
    }

    /* 这里只处理Post请求, Get请求，就直接return 200 OK  */
    if (evhttp_request_get_command (req) != EVHTTP_REQ_POST)
    { 
        evhttp_send_reply (req, 200, "OK", NULL);
        return;
    }

    printf ("Got a POST request for <%s>\n", uri);
    LOG (LOG_MODULE, LOG_PROC_SETORDER, "Got a POST request for <%s>\n", uri);

    //判断此URI是否合法
    decoded = evhttp_uri_parse (uri);
    if (! decoded)
    { 
        printf ("It's not a good URI. Sending BADREQUEST\n");
        LOG (LOG_MODULE, LOG_PROC_SETORDER, "It's not a good URI. Sending BADREQUEST\n");
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
    LOG(LOG_MODULE, LOG_PROC_SETORDER, "[post_data][%d]=\n %s\n", post_data_len, payload);


    /*
       具体的：可以根据Post的参数执行相应操作，然后将结果输出
       ...
     */
    //=======================================================
    //unpack json
    cJSON* root = cJSON_Parse(request_data_buf);
    cJSON* sessionid = cJSON_GetObjectItem(root, "sessionid");
    cJSON* driver = cJSON_GetObjectItem(root, "driver");
    cJSON* src_longitude = cJSON_GetObjectItem(root, "src_longitude");
    cJSON* src_latitude = cJSON_GetObjectItem(root, "src_latitude");
    cJSON* src_address = cJSON_GetObjectItem(root, "src_address");
    cJSON* dst_longitude = cJSON_GetObjectItem(root, "dst_longitude");
    cJSON* dst_latitude = cJSON_GetObjectItem(root, "dst_latitude");
    cJSON* dst_address = cJSON_GetObjectItem(root, "dst_address");
    cJSON* RMB = cJSON_GetObjectItem(root, "RMB");
    cJSON* create_order_time = cJSON_GetObjectItem(root, "create_order_time");
    

#if 1
    printf("sessionid = %s\n", sessionid->valuestring);
    printf("driver = %s\n", driver->valuestring);
    printf("src_longitude= %s\n", src_longitude->valuestring);
    printf("src_latitude= %s\n", src_latitude->valuestring);
    printf("src_address = %s\n", src_address->valuestring);
    printf("dst_longitude= %s\n", dst_longitude->valuestring);
    printf("dst_latitude= %s\n", dst_latitude->valuestring);
    printf("dst_address = %s\n", dst_address->valuestring);
    printf("RMB= %s\n", RMB->valuestring);
    printf("create_order_time = %s\n", create_order_time->valuestring);
#endif

    geo_drvier_t *drivers = NULL;
    int driver_count = 0;
    char orderid[ORDERID_STR_LEN] = {0};

    //得到所有附近司机数据
    ret = curl_to_cacheserver_radiusGeo(KEY_ONLINE_DRIVERS_GEO_ZSET, src_longitude->valuestring, src_latitude->valuestring, GEO_RADIUS, &drivers, &driver_count);
    //drivers need free



    char* recode = RECODE_OK;

    if (ret == 0) {

        if (driver_count == 0) {
            //没有找到适当司机,使用户重新约车,或者等待约车
            recode = RECODE_NO_DRIVER;
            ret = -1;
        }
        else {
            //生成订单号
            create_orderid(orderid);

            //创建临时订单
            ret = curl_to_cacheserver_create_order(orderid,
                                                   sessionid->valuestring,
                                                   drivers[0].sessionid,
                                                   create_order_time->valuestring,
                                                   src_address->valuestring,
                                                   dst_address->valuestring,
                                                   src_longitude->valuestring,
                                                   src_latitude->valuestring,
                                                   dst_longitude->valuestring,
                                                   dst_latitude->valuestring,
                                                   RMB->valuestring);

        }

    }

    if (ret == 0) {
        //给司机的sessionid添加orderid
        ret = curl_to_cacheserver_set_orderid(drivers[0].sessionid, orderid);
    }

    if (ret == 0) {
        //给乘客的sessionid添加orderid
        ret = curl_to_cacheserver_set_orderid(sessionid->valuestring, orderid);
    }

    //将sessionid存放到缓存数据库中
    char *response_data = make_gen_res_json(ret, recode, "get radiusGeo ERROR");

    cJSON_Delete(root);
    if (drivers != NULL) {
        free(drivers);
    }
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
    LOG(LOG_MODULE, LOG_PROC_SETORDER, "[response]:\n");
    LOG(LOG_MODULE, LOG_PROC_SETORDER, "%s\n", response_data);

    free(response_data);
}
