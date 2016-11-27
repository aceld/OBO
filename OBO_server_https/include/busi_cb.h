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
 * @brief  远程注册用户请求业务
 *
 * @param username
 * @param password
 * @param tel
 * @param email
 * @param driver    "yes" or "no"
 * @param id_card       
 * @param sessionid OUT 得到的sessionid
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
                           const char* id_card,
                           char *sessionid);


/* -------------------------------------------*/
/**
 * @brief  远程查询登陆信息表
 *
 * @param username
 * @param password
 * @param isDriver
 * @param sessionid OUT得到的sessionid
 *
 * @returns   
 *  0 succ  -1 fail
 */
/* -------------------------------------------*/
int curl_to_dataserver_login(const char *username,
                             const char *password,
                             const char *isDriver,
                             char *sessionid);



/* -------------------------------------------*/
/**
 * @brief  远程存储SESSIONID - id_card  临时
 *
 * @param username
 * @param sessionid 
 *
 * @returns   
 *          0 succ, -1 fail
 */
/* -------------------------------------------*/
int curl_to_cacheserver_session(const char *username,  const char* sessionid);


#endif
