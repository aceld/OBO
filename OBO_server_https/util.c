#include "util.h"
#include "cJSON.h"

char *get_random_uuid(char *str)
{
    uuid_t uuid;

    uuid_generate(uuid);
    uuid_unparse(uuid, str);

    return str;
}

char *make_reg_login_res_json(int ret, char *sessionid, char *reason)
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

    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);

    return response_data;
}
