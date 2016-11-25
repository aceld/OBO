#include "redis_op.h"
#include "tables.h"
#include "util.h"
#include "cJSON.h"


int deal_cache(char *request_data_buf)
{
    int ret = 0;
    //unpack json
    cJSON* root     = cJSON_Parse(request_data_buf);
    cJSON* cmd      = cJSON_GetObjectItem(root, "cmd");

    //入库redis数据库
    if (strcmp(cmd->valuestring, KEY_CMD_SETSTRING) == 0) {
        //设置 String类型的 key-value键值对

        cJSON* key      = cJSON_GetObjectItem(root, "key");
        cJSON* value    = cJSON_GetObjectItem(root, "value");
        cJSON* unlimited= cJSON_GetObjectItem(root, "unlimited");

        printf("cmd = %s\n", cmd->valuestring);
        printf("key = %s\n", key->valuestring);
        printf("value = %s\n", value->valuestring);
        printf("unlimited = %s\n", unlimited->valuestring);


        redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);
        if (conn == NULL) {
            printf("redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
            ret = -1;
            goto END;
        
        }
        
        ret = rop_set_string(conn, key->valuestring, value->valuestring);


        if (strcmp(unlimited->valuestring, "no") == 0) {
            cJSON* lifecycle= cJSON_GetObjectItem(root, "lifecycle");
            printf("lifecycle = %d\n", lifecycle->valueint);
            //给该key设置声明周期
            rop_set_key_lifetime(conn, key->valuestring, lifecycle->valueint);
        }

        rop_disconnect(conn);
    }

END:
    cJSON_Delete(root);

    return ret;

}
