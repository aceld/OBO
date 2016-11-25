#ifndef  __BUSI_CB_H_
#define  __BUSI_CB_H_

#define MYHTTPD_SIGNATURE   "OBOHttpd v0.1"

void login_cb (struct evhttp_request *req, void *arg);
void reg_cb (struct evhttp_request *req, void *arg);

#endif
