#ifndef _UTIL_H_
#define _UTIL_H_


#include <uuid/uuid.h>

#define UUID_STR_LEN        (36)
#define URI_DATA_SERVER_PER "https://101.200.190.150:18889/persistent"
#define URI_DATA_SERVER_CHE "https://101.200.190.150:18889/cache"
#define RESPONSE_DATA_LEN  (4096)


typedef struct curl_response_data
{
    char data[RESPONSE_DATA_LEN];
    int data_len;

}curl_response_data_t;


char *get_random_uuid(char *str);
char *make_reg_login_res_json(int ret, char *reason);

#endif
