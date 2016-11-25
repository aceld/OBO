#ifndef  __BUSI_CB_H_
#define  __BUSI_CB_H_

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#define MYHTTPD_SIGNATURE   "OBOHttpd v0.1"

void login_cb (struct evhttp_request *req, void *arg);
void reg_cb (struct evhttp_request *req, void *arg);



/* -------------------------------------------*/
/**
 * @brief  远程存储注册信息表 持久
 *
 * @param request_data_buf
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_dataserver_reg(char* request_data_buf);




/* -------------------------------------------*/
/**
 * @brief  远程存储SESSIONID - id_card  临时
 *
 * @param request_data_buf
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_session(char *request_data_buf, char* sessionid);

#endif
