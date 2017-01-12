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
#include "make_log.h"

#define TIME_STR_LEN                    (32)
#define UUID_STR_LEN                    (36)
#define SESSIONID_STR_LEN               (64)
#define ORDERID_STR_LEN                 (64)
#define DISTANCE_STR_LEN                (20)
#define LOCATION_POINT_STR_LEN          (20)
#define ADDRESS_STR_LEN                 (256)
#define RESPONSE_DATA_LEN               (4096)
#define USERNAME_STR_LEN                (256)

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

#define STATUS_DRIVER_IDLE              "idel"
#define STATUS_DRIVER_CATCH              "catching"
#define STATUS_DRIVER_READY              "ready"
#define STATUS_DRIVER_DRIVE              "driving"
#define STATUS_PASSENGER_IDLE           "idle"
#define STATUS_PASSENGER_WAIT           "waiting"
#define STATUS_PASSENGER_TRAVEL           "traveling"

#define LOG_MODULE                      "OBO_BUSI"
#define LOG_PROC_LOGIN                  "login"
#define LOG_PROC_REG                    "login"
#define LOG_PROC_SETORDER               "set_order"
#define LOG_PROC_FINISHORDER            "finish_order"
#define LOG_PROC_REMOTE_CURL            "remote_curl"

typedef struct curl_response_data
{
    char data[RESPONSE_DATA_LEN];
    int data_len;

}curl_response_data_t;


void get_time_str( char* time_str );
char *get_random_uuid(char *str);
char * create_sessionid(const char *isDriver, char *sessionid);
char * create_orderid(char *orderid);

//返回前端json数据包封装
char *make_reg_login_res_json(int ret, char* recode, char *sessionid, char *reason);
char *make_gen_res_json(int ret, char* recode, char *reason);
char *make_driver_locationChanged_res_json(int ret, char *recode, char* status, char *orderid, char *reason, char *ptemp_longitude, char *ptemp_latitude);
char *make_passenger_locationChanged_res_json(int ret, char *recode, char* status, char *orderid, char *reason, char *dtemp_longitude, char *dtemp_latitude, char *order_status);

typedef struct geo_drvier
{
    char sessionid[SESSIONID_STR_LEN];
    char distance[DISTANCE_STR_LEN];
    char longitude[LOCATION_POINT_STR_LEN];
    char latitude[LOCATION_POINT_STR_LEN];
} geo_drvier_t;

typedef struct order
{
    char orderid[ORDERID_STR_LEN];

    char passenger_username[ORDERID_STR_LEN];
    char driver_username[ORDERID_STR_LEN];

    char create_order_time[TIME_STR_LEN];
    char start_order_time[TIME_STR_LEN];
    char end_time[TIME_STR_LEN];

    char src_address[ADDRESS_STR_LEN];
    char dst_address[ADDRESS_STR_LEN];
    char src_longitude[LOCATION_POINT_STR_LEN];
    char src_latitude[LOCATION_POINT_STR_LEN];
    char dst_longitude[LOCATION_POINT_STR_LEN];
    char dst_latitude[LOCATION_POINT_STR_LEN];

    char src_address_real[ADDRESS_STR_LEN];
    char dst_address_real[ADDRESS_STR_LEN];
    char src_longitude_real[LOCATION_POINT_STR_LEN];
    char src_latitude_real[LOCATION_POINT_STR_LEN];
    char dst_longitude_real[LOCATION_POINT_STR_LEN];
    char dst_latitude_real[LOCATION_POINT_STR_LEN];

    char RMB[64];

}order_t;

#define ORDER_INFO_LEN      sizeof(struct order)


void login_cb (struct evhttp_request *req, void *arg);
void reg_cb (struct evhttp_request *req, void *arg);
void set_order_cb (struct evhttp_request *req, void *arg);
void locationChanged_cb (struct evhttp_request *req, void *arg);
void finish_order_cb (struct evhttp_request *req, void *arg);



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
 * @brief  创建订单 持久型
 *
 * @param order
 *
 * @returns   
 *    0 succ    -1 fail
 */
/* -------------------------------------------*/
int curl_to_dataserver_create_order(order_t *order);


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
 * @brief  设置订单首次启动 相关数据
 *
 * @param orderid
 * @param order_status
 * @param start_driving_time
 * @param src_address_real
 * @param src_longitude_real
 * @param src_latitude_real
 * @param dtemp_longitude
 * @param dtmp_latitude
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_startorder_real(char *orderid, char *order_status, char *start_driving_time,char *src_address_real, char *src_longitude_real, char *src_latitude_real);

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
int curl_to_cacheserver_get_orderid (const char *sessionid, char *order_id) ;

/* -------------------------------------------*/
/**
 * @brief  判读sessionid是否存在
 *
 * @param sessionid
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_existsessionid(const char *sessionid);


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
int curl_to_cacheserver_remGeo(const char *sessionid);


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
int curl_to_cacheserver_addGeo(const char *sessionid, const char *longitude, const char *latitude) ;


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
int curl_to_cacheserver_set_dtemp_location(const char *orderid, const char *dtemp_longitude, const char *dtemp_latitude) ;


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
int curl_to_cacheserver_set_ptemp_location(const char *orderid, const char *ptemp_longitude, const char *ptemp_latitude) ;


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
int curl_to_cacheserver_get_dtemp_location(const char *orderid, char *dtemp_longitude, char *dtemp_latitude);


/* -------------------------------------------*/
/**
 * @brief  获取当前乘客的临时坐标地址信息
 *
 * @param orderid
 * @param dtemp_longitude   OUT
 * @param dtemp_latitude    OUT
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_get_ptemp_location(const char *orderid, char *ptemp_longitude, char *ptemp_latitude);

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
int curl_to_cacheserver_get_orderd ( const char *orderid, char*passenger_sessionid, char *driver_sessionid, order_t *order_info);

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
int curl_to_cacheserver_get_order_status(const char *orderid, char *order_status);

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
int curl_to_cacheserver_get_username(const char *sessionid, char *username);

/* -------------------------------------------*/
/**
 * @brief  删除一个缓存表
 *
 * @param key
 *
 * @returns   
 */
/* -------------------------------------------*/
int curl_to_cacheserver_delete_key(const char *key);

#endif
