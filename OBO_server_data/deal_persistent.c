#include "dao_mysql.h"
#include "tables.h"
#include "util.h"
#include "cJSON.h"


int insert_table_user(char *username, char *password, char *tel,
                      char *email, char *id_card, char *driver)
{
    int ret = 0;
    char query[SQL_MAX_LEN] = {0};

    MYSQL *conn = msql_conn(g_db_config.db_username,
                            g_db_config.db_passwd,
                            g_db_config.db_basename);
    if (conn == NULL) {
        printf("====== conn mysql error!=====\n");
        return -1;
    }

    sprintf(query, 
            "insert into %s (u_name, password, phone, email, id_card, driver) values ('%s', '%s', '%s', '%s', '%s', '%s')", 
            TABLE_USER , username, password, tel, email, id_card, driver);

    if (mysql_query(conn, query)) {
        printf("query = %s\n", query);
        printf("==== insert %s error===\n", TABLE_USER);
        print_error(conn, "insert");
        ret = -1;
    }

    mysql_close(conn);

    return ret;
}


int deal_persistent(char *request_data_buf)
{
    int ret = 0;
    //unpack json
    cJSON* root     = cJSON_Parse(request_data_buf);
    cJSON* cmd      = cJSON_GetObjectItem(root, "cmd");
    cJSON* table    = cJSON_GetObjectItem(root, "table");

    if (strcmp(cmd->valuestring, TABLE_CMD_INSERT) == 0) {

        //入库msql数据库
        if (strcmp(table->valuestring, TABLE_USER) == 0) {

            cJSON* username = cJSON_GetObjectItem(root, "username");
            cJSON* password = cJSON_GetObjectItem(root, "password");
            cJSON* tel      = cJSON_GetObjectItem(root, "tel");
            cJSON* email    = cJSON_GetObjectItem(root, "email");
            cJSON* driver   = cJSON_GetObjectItem(root, "driver");
            cJSON* id_card  = cJSON_GetObjectItem(root, "id_card");

            printf("cmd = %s\n", cmd->valuestring);
            printf("table = %s\n", table->valuestring);
            printf("username = %s\n", username->valuestring);
            printf("password = %s\n", password->valuestring);
            printf("tel = %s\n", tel->valuestring);
            printf("email = %s\n", email->valuestring);
            printf("driver = %s\n", driver->valuestring);
            printf("id_card = %s\n", id_card->valuestring);

            ret = insert_table_user(username->valuestring,
                                 password->valuestring,
                                  tel->valuestring,
                                  email->valuestring,
                                  id_card->valuestring,
                                  driver->valuestring);
        }

    }



    cJSON_Delete(root);

    return ret;

}
