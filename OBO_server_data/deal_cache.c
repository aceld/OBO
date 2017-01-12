/**
 * @file deal_cache.c
 * @brief  处理缓存型数据库存储业务
 * @author liu_danbing <danbing_at@163.com>
 * @version 1.0
 * @date 2016-11-25
 */
#include "redis_op.h"
#include "tables.h"
#include "util.h"
#include "cJSON.h"

int deal_cache_setLifecycle(cJSON *root)
{
    /*
        {
            cmd:     "setLifcycle",
            key:     "online-driver-[sessionid]",
            lifecycle: 600
        }
     */
        int ret = 0;

        cJSON* key          = cJSON_GetObjectItem(root, "key");
        cJSON* lifecycle    = cJSON_GetObjectItem(root, "lifecycle");
        
        LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "key = %s, lifecycle = %d\n", key->valuestring, lifecycle->valueint);

        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }
        //给该key设置声明周期
        ret = rop_set_key_lifetime(conn, key->valuestring, lifecycle->valueint);

END:
        rop_disconnect(conn);
        return ret;
}

/*
        处理setString 指令 
 */
int deal_cache_setString(cJSON *root)
{
        int ret = 0;
        cJSON* key      = cJSON_GetObjectItem(root, "key");
        cJSON* value    = cJSON_GetObjectItem(root, "value");
        cJSON* unlimited= cJSON_GetObjectItem(root, "unlimited");

        LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "key = %s\n", key->valuestring);
        LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "value = %s\n", value->valuestring);
        LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "unlimited = %s\n", unlimited->valuestring);


        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }
        
        ret = rop_set_string(conn, key->valuestring, value->valuestring);


        if (strcmp(unlimited->valuestring, "no") == 0) {
            cJSON* lifecycle= cJSON_GetObjectItem(root, "lifecycle");
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "lifecycle = %d\n", lifecycle->valueint);
            //给该key设置声明周期
            rop_set_key_lifetime(conn, key->valuestring, lifecycle->valueint);
        }

END:
        rop_disconnect(conn);
        return ret;
}

/*
        判断key是否存在
 */
int deal_cache_existKey(cJSON *root)
{

    /*
        {
            cmd:     "existKey",
            key:     "online-driver-[sessionid]",
        }
     */
        int ret = 0;

        cJSON* key          = cJSON_GetObjectItem(root, "key");
        
        LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "key = %s\n", key->valuestring);

        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }

        //判断key是否存在
        if (rop_is_key_exist(conn, key->valuestring) == 1) {
            ret = 0;
        }
        else {
            ret = -1;
        }

END:
        rop_disconnect(conn);
        return ret;
}


/*
        处理setHash 指令 
 */
int deal_cache_setHash(cJSON *root)
{
        int ret = 0;
        int i = 0;

        /*
            cmd:     "setHash",
            key:     "online-driver-[sessionid]",
            fields:   ["username","orderid"],
            values:   ["盖伦", "NONE"]
         */
        cJSON* key      = cJSON_GetObjectItem(root, "key");
        cJSON* fields   = cJSON_GetObjectItem(root, "fields");
        cJSON* values   = cJSON_GetObjectItem(root, "values");
        int array_size = cJSON_GetArraySize(fields);

        RFIELDS rfields  = malloc (FIELD_ID_SIZE *array_size);
        RVALUES rvalues = malloc(VALUES_ID_SIZE *array_size);



        for (i = 0;i < array_size; i++) {
            strncpy(rfields[i], cJSON_GetArrayItem(fields, i)->valuestring, FIELD_ID_SIZE-1);
            strncpy(rvalues[i], cJSON_GetArrayItem(values, i)->valuestring, VALUES_ID_SIZE-1);
        }


        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }
        
        ret = rop_hash_set_append(conn, key->valuestring, rfields, rvalues, array_size);


END:
        rop_disconnect(conn);
        if (rfields != NULL) {
            free(rfields);
        }
        if (rvalues != NULL) {
            free(rvalues);
        }
        return ret;
}

/*
        处理getHash 指令 
 */
int deal_cache_getHash(cJSON *root, RVALUES *rvalues_p, int *value_num_p)
{
        int ret = 0;
        int i = 0;

        /*
            cmd:     "getHash",
            key:     "online-driver-[sessionid]",
            fields:   ["username","orderid"]
         */
        cJSON* key      = cJSON_GetObjectItem(root, "key");
        cJSON* fields   = cJSON_GetObjectItem(root, "fields");
        int array_size = cJSON_GetArraySize(fields);

        RFIELDS rfields  = malloc (FIELD_ID_SIZE *array_size);
        RVALUES rvalues  = malloc (VALUES_ID_SIZE *array_size);

        for (i = 0;i < array_size; i++) {
            strncpy(rfields[i], cJSON_GetArrayItem(fields, i)->valuestring, FIELD_ID_SIZE-1);
        }


        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }
        
        ret = rop_hash_get_append(conn, key->valuestring, rfields, rvalues, array_size);

        for(i = 0; i < array_size; i++) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "=====> field : %s, value: %s\n", rfields[i], rvalues[i]);
        }
        
        *rvalues_p = rvalues;
        *value_num_p = array_size;

END:
        rop_disconnect(conn);
        if (rfields != NULL) {
            free(rfields);
        }
        return ret;
}

/*
        处理radiusGeo 指令 
 */
int deal_cache_radiusGeo(cJSON *root, RGEO *geo_array_p, int *geo_num_p)
{
        int ret = 0;


        /*
           {
               cmd:      "radiusGeo",
               key:      "ONLINE_DRIVER_GEO_ZSET",
               longitude:  "98.123123123",
               latitude:   "39.123123123",
               radius:    "200"  //方圆多少米
            }
         */
        cJSON* key        = cJSON_GetObjectItem(root, "key");
        cJSON* longitude  = cJSON_GetObjectItem(root, "longitude");
        cJSON* latitude   = cJSON_GetObjectItem(root, "latitude");
        cJSON* radius     = cJSON_GetObjectItem(root, "radius");



        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }



        ret = rop_geo_radius(conn, key->valuestring, longitude->valuestring, latitude->valuestring, radius->valuestring, geo_array_p, geo_num_p);


END:
        rop_disconnect(conn);
        return ret;
}

/*
        处理remZset 指令 
 */
int deal_cache_remZset(cJSON *root)
{
        int ret = 0;


        /*
           {
               cmd:      "remZset",
               key:      "ONLINE_DRIVER_GEO_ZSET",
               member:  "online-driver-[sessionid]",
            }
         */
        cJSON* key        = cJSON_GetObjectItem(root, "key");
        cJSON* member  = cJSON_GetObjectItem(root, "member");



        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }


        ret = rop_zset_rem_member(conn, key->valuestring, member->valuestring);

        ret = 0;//删除成功与否不做判断

END:
        rop_disconnect(conn);
        return ret;
}


/*
        处理addGeo 指令 
 */
int deal_cache_addGeo(cJSON *root)
{
        int ret = 0;


        /*
           {
               cmd:      "addGeo",
               key:      "ONLINE_DRIVER_GEO_ZSET",
               member:  "online-driver-[sessionid]",
                longitude: "98.11",
                latitude: "98.11"
            }
         */
        cJSON* key        = cJSON_GetObjectItem(root, "key");
        cJSON* member  = cJSON_GetObjectItem(root, "member");
        cJSON* longitude  = cJSON_GetObjectItem(root, "longitude");
        cJSON* latitude  = cJSON_GetObjectItem(root, "latitude");




        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }


        ret = rop_add_geo(conn, key->valuestring, member->valuestring, longitude->valuestring, latitude->valuestring);


END:
        rop_disconnect(conn);
        return ret;
}


/*
        处理deleteKey 指令 
 */
int deal_cache_deleteKey(cJSON *root)
{
        int ret = 0;


        /*
           {
               cmd:      "deleteKey",
               key:      "orderid-xxx-xx-x-x-xx-x"
            }
         */
        cJSON* key        = cJSON_GetObjectItem(root, "key");


        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }


        ret = rop_del_key(conn, key->valuestring);


END:
        rop_disconnect(conn);

        return ret;
}


char * deal_cache(const char *request_data_buf)
{
    char *response_data;
    int ret = 0;
    //unpack json
    cJSON* root     = cJSON_Parse(request_data_buf);
    cJSON* cmd      = cJSON_GetObjectItem(root, "cmd");

    LOG(LOG_MODULE_SERVER_DATA, LOG_PROC_CACHE, "cmd = %s\n", cmd->valuestring);
    //入库redis数据库
    if (strcmp(cmd->valuestring, KEY_CMD_SETSTRING) == 0) {
        //设置 String类型的 key-value键值对
        ret = deal_cache_setString(root);

        response_data = make_response_json(ret, "set string ERROR");
    }

    else if (strcmp(cmd->valuestring, KEY_CMD_SETHASH) == 0) {
        //设置 Hash类型的 数据
        ret = deal_cache_setHash(root);
        
        response_data = make_response_json(ret, "set hash ERROR");
    }

    else if (strcmp(cmd->valuestring, KEY_CMD_SETLIFECYCLE) == 0) {
        //设置 key 的生命周期
        ret = deal_cache_setLifecycle(root);

        response_data = make_response_json(ret, "set lifecycle ERROR");
    }
    else if (strcmp(cmd->valuestring, KEY_CMD_GETHASH) == 0) {
        //获取 hash的 字段值
        RVALUES rvalues = NULL;
        int value_num = 0;
        ret = deal_cache_getHash(root, &rvalues, &value_num);

        response_data = make_response_gethash_json(ret, "get hash field ERROR", rvalues, value_num);
        if (rvalues != NULL) {
            free(rvalues);
        }
    }
    else if (strcmp(cmd->valuestring, KEY_CMD_RADIUSGEO) == 0) {
        //获取 周边范围内地理位置信息

        RGEO geo_array = NULL;
        int geo_num = 0;

        ret = deal_cache_radiusGeo(root, &geo_array, &geo_num);

        response_data = make_response_geo_drivers_json(ret, "radiusGeo error", geo_array, geo_num);

        if (geo_array != NULL) {
            free(geo_array);
        }
    }
    else if (strcmp(cmd->valuestring, KEY_CMD_EXIST) == 0) {
        //判断 key 是否存在

        ret = deal_cache_existKey(root);

        response_data = make_response_json(ret, "sessionid not exist ERROR");
        
    }
    else if (strcmp(cmd->valuestring, KEY_CMD_REMZSET) == 0) {
        //删除 zset中的member
        ret = deal_cache_remZset(root);
        response_data = make_response_json(ret, "rem zset ERROR");
    }
    else if (strcmp(cmd->valuestring, KEY_CMD_ADDGEO) == 0) {
        // 添加 一个 地理位置信息 
        ret = deal_cache_addGeo(root);
        response_data = make_response_json(ret, "add Geo ERROR");
    }
    else if (strcmp(cmd->valuestring, KEY_CMD_DELETE) == 0) {
        //删除key
        ret = deal_cache_deleteKey(root);
        response_data = make_response_json(ret, "delete Key ERROR");
    }
    else {
        response_data = make_response_json(-1, "unknow cache CMD");
    }
    

    cJSON_Delete(root);

    return response_data;
}
