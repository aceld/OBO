#include <stdio.h>
#include <string.h>
#include "util.h"
#include "cJSON.h"
#include "busi_cb.h"

void get_time_str( char* time_str )
{
    time_t t = time(NULL);
    strftime( time_str, TIME_STR_LEN, "%Y-%m-%d %X",localtime(&t) );
}

char *get_random_uuid(char *str)
{
    uuid_t uuid;

    uuid_generate(uuid);
    uuid_unparse(uuid, str);

    return str;
}

char * create_orderid(char *orderid)
{
    char uuid[UUID_STR_LEN] = {0};

    sprintf(orderid, "orderid-%s", get_random_uuid(uuid));

    return orderid;
}


char * create_sessionid(const char *isDriver, char *sessionid)
{
    char uuid[UUID_STR_LEN] = {0};

    if (strcmp(isDriver, "yes") == 0) {
        sprintf(sessionid, "online-driver-%s", get_random_uuid(uuid));
    }
    else {
        sprintf(sessionid, "online-user-%s", get_random_uuid(uuid));
    }

    return sessionid;
}

char *make_passenger_locationChanged_res_json(int ret, char *recode, char* status, char *orderid, char *reason, char *dtemp_longitude, char *dtemp_latitude, char *order_status)
{
    //packet json
    cJSON *root = cJSON_CreateObject();

    if (ret == 0) {
        cJSON_AddStringToObject(root, "result", "ok");
        cJSON_AddStringToObject(root, "status", status);
        cJSON_AddStringToObject(root, "orderid", orderid);
        cJSON_AddStringToObject(root, "dtemp_longitude", dtemp_longitude);
        cJSON_AddStringToObject(root, "dtemp_latitude", dtemp_latitude);
        if (strcmp(status, STATUS_PASSENGER_WAIT) == 0)  {
            cJSON_AddStringToObject(root, "order_status", order_status);
        }
    }
    else {
        cJSON_AddStringToObject(root, "result", "error");
        cJSON_AddStringToObject(root, "reason", reason);
    }
    cJSON_AddStringToObject(root, "recode", recode);

    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);

    return response_data;
}

char *make_driver_locationChanged_res_json(int ret, char *recode, char* status, char *orderid, char *reason, char *ptemp_longitude, char *ptemp_latitude)
{
    //packet json
    cJSON *root = cJSON_CreateObject();

    if (ret == 0) {
        cJSON_AddStringToObject(root, "result", "ok");
        cJSON_AddStringToObject(root, "status", status);
        cJSON_AddStringToObject(root, "orderid", orderid);
        if (strcmp(status, STATUS_DRIVER_CATCH) == 0 ||
                strcmp(status, STATUS_DRIVER_DRIVE) == 0) {
            cJSON_AddStringToObject(root, "ptemp_longitude", ptemp_longitude);
            cJSON_AddStringToObject(root, "ptemp_latitude", ptemp_latitude);
        }
    }
    else {
        cJSON_AddStringToObject(root, "result", "error");
        cJSON_AddStringToObject(root, "reason", reason);
    }
    cJSON_AddStringToObject(root, "recode", recode);

    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);

    return response_data;
}

char *make_gen_res_json(int ret, char* recode, char *reason)
{
    //packet json
    cJSON *root = cJSON_CreateObject();

    if (ret == 0) {
        cJSON_AddStringToObject(root, "result", "ok");

    }
    else {
        cJSON_AddStringToObject(root, "result", "error");
        cJSON_AddStringToObject(root, "reason", reason);
    }
    cJSON_AddStringToObject(root, "recode", recode);

    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);

    return response_data;
}

char *make_reg_login_res_json(int ret, char* recode, char *sessionid, char *reason)
{
    //packet json
    cJSON *root = cJSON_CreateObject();

    if (ret == 0) {
        cJSON_AddStringToObject(root, "result", "ok");
        cJSON_AddStringToObject(root, "sessionid", sessionid);
    }
    else {
        cJSON_AddStringToObject(root, "result", "error");
        cJSON_AddStringToObject(root, "reason", reason);
    }
    cJSON_AddStringToObject(root, "recode", recode);

    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);

    return response_data;
}
