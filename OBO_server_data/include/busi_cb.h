#ifndef __BUSI_CB_H_
#define  __BUSI_CB_H_

#define MYHTTPD_SIGNATURE   "MoCarHttpd v0.1"

void persistent_store_cb (struct evhttp_request *req, void *arg);
void cache_store_cb (struct evhttp_request *req, void *arg);



char* deal_persistent(char *request_data_buf);
char* deal_cache(char *request_data_buf);

#endif
