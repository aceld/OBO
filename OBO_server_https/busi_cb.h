#ifndef __BUSI_CB_H_
#define  __BUSI_CB_H_

#define MYHTTPD_SIGNATURE   "MoCarHttpd v0.1"
#define UUID_STR_LEN        (36)

void login_cb (struct evhttp_request *req, void *arg);

#endif
