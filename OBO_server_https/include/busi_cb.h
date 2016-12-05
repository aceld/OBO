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



#include <uuid/uuid.h>

#define UUID_STR_LEN                    (36)
#define SESSIONID_STR_LEN               (64)
#define ORDERID_STR_LEN                 (64)
#define DISTANCE_STR_LEN                (20)
#define LOCATION_POINT_STR_LEN          (20)
#define RESPONSE_DATA_LEN               (4096)

#define URI_DATA_SERVER_PER             "https://101.200.190.150:18889/persistent"
#define URI_DATA_SERVER_CHE             "https://101.200.190.150:18889/cache"

#define SESSIONID_LIFECYCLE             (600)
#define ORDER_ID_NONE                   "NONE"


#define KEY_ONLINE_DRIVERS_GEO_ZSET     "ONLINE_DRIVER_GEO_ZSET"
#define GEO_RADIUS                      "200"

#define RECODE_OK                       "0"
#define RECODE_SESSION_ERROR            "1"
#define RECODE_SERVER_ERROR             "2"
#define RECODE_NO_DRIVER                "3"

typedef struct curl_response_data
{
    char data[RESPONSE_DATA_LEN];
    int data_len;

}curl_response_data_t;


char *get_random_uuid(char *str);
char * create_sessionid(const char *isDriver, char *sessionid);
char * create_orderid(char *orderid);
char *make_reg_login_res_json(int ret, char* recode, char *sessionid, char *reason);
char *make_gen_res_json(int ret, char* recode, char *reason);

typedef struct geo_drvier
{
    char sessionid[SESSIONID_STR_LEN];
    char distance[DISTANCE_STR_LEN];
    char longitude[LOCATION_POINT_STR_LEN];
    char latitude[LOCATION_POINT_STR_LEN];
} geo_drvier_t;


void login_cb (struct evhttp_request *req, void *arg);
void reg_cb (struct evhttp_request *req, void *arg);
void set_order_cb (struct evhttp_request *req, void *arg);



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
                           const char* id_card);


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
int curl_to_cacheserver_radiusGeo(const char *key, const char *longitude, const char *latitude,const char *radius, geo_drvier_t **geo_drivers_p, int *driver_count);

/* -------------------------------------------*/
/**
 * @brief  远程查询登陆信息表
 *
 * @param username
 * @param password
 * @param isDriver
 *
 * @returns   
 *  0 succ  -1 fail
 */
/* -------------------------------------------*/
int curl_to_dataserver_login(const char *username,
                             const char *password,
                             const char *isDriver);



/* -------------------------------------------*/
/**
 * @brief  远程存储SESSIONID - id_card  临时
 *
 * @param username
 * @param sessionid 
 * @param orderid 
 *
 * @returns   
 *          0 succ, -1 fail
 */
/* -------------------------------------------*/
int curl_to_cacheserver_session(const char *username,  const char* sessionid, const char *orderid);

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
int curl_to_cacheserver_lifecycle(const char *sessionid, int lifecycle);


/* -------------------------------------------*/
/**
 * @brief  创建临时订单
 *
 * @param orderid
 * @param passenger_sessionid
 * @param driver_sessionid
 * @param create_order_time
 * @param src_address
 * @param dst_address
 * @param src_longitude
 * @param src_latitude
 * @param dst_longitude
 * @param dst_latitude
 * @param RMB
 *
 * @returns   
 */
/* -------------------------------------------*/
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
                                     const char *RMB);


/* -------------------------------------------*/
/**
 * @brief  给一个sessionID 设置一个orderid字段
 *
 * @param sessionid
 * @param orderid
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_set_orderid (const char *sessionid,
                                     const char *orderid);
#endif
