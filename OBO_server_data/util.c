#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "cJSON.h"



db_config_t g_db_config;



void config_init()
{ 
   GetProfileString(CONFIG_PATH, "redis", "ip", g_db_config.cache_ip);
   GetProfileString(CONFIG_PATH, "redis", "port", g_db_config.cache_port);

   GetProfileString(CONFIG_PATH, "mysql", "ip", g_db_config.db_ip);
   GetProfileString(CONFIG_PATH, "mysql", "port", g_db_config.db_port);
   GetProfileString(CONFIG_PATH, "mysql", "database", g_db_config.db_basename);
   GetProfileString(CONFIG_PATH, "mysql", "user", g_db_config.db_username);
   GetProfileString(CONFIG_PATH, "mysql", "passwd", g_db_config.db_passwd);
}

char *make_response_json(int ret, char *reason)
{
    cJSON *root = cJSON_CreateObject();
    if (ret == 0) {
        cJSON_AddStringToObject(root, "result", "ok");
    }
    else {
        cJSON_AddStringToObject(root, "result", "error");
        cJSON_AddStringToObject(root, "reason", reason);
    }
    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);

    return response_data;
}



