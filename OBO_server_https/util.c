#include "util.h"
#include "cJSON.h"

char *get_random_uuid(char *str)
{
    uuid_t uuid;

    uuid_generate(uuid);
    uuid_unparse(uuid, str);

    return str;
}

char *make_reg_login_res_json(int ret, char *reason)
{
    //packet json
    cJSON *root = cJSON_CreateObject();

    if (ret == 0) {

        cJSON_AddStringToObject(root, "result", "ok");
        /* 生成uuid随机的 */
        char uuid_str[UUID_STR_LEN] = {0};
        cJSON_AddStringToObject(root, "sessionid", get_random_uuid(uuid_str));

    }
    else {
        cJSON_AddStringToObject(root, "result", "error");
        cJSON_AddStringToObject(root, "reason", reason);
    }

    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);

    return response_data;
}
