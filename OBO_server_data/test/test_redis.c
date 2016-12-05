
#include "config.h"
#include "util.h"
#include "redis_op.h"

int main(void)
{
    int ret = 0;

    config_init();

    redisContext* conn = rop_connectdb_nopwd(g_db_config.cache_ip,
                                                 g_db_config.cache_port);

    if (conn == NULL) {
        printf("redis connect error %s:%s\n", g_db_config.cache_ip, g_db_config.cache_port); 
        ret = -1;
        goto END;
    
    }
    RGEO geo_array = NULL;
    int geo_num = 0;
    int i = 0;

    ret = rop_geo_radius(conn, "Sicily", "15.123123", "37.123123", "300", &geo_array, &geo_num);
#if 1
    printf("geo_num = %d\n", geo_num);
    for (i = 0; i < geo_num; i++) {
        printf("name = %s\n", geo_array[i].name);
        printf("distance = %s\n", geo_array[i].distance);
        printf("longitude = %s\n", geo_array[i].longitude);
        printf("latitude = %s\n", geo_array[i].latitude);
    }
#endif

    free(geo_array);
END:
    rop_disconnect(conn);
    return ret;
}
