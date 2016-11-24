#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
//for struct evkeyvalq
#include <sys/queue.h>
#include <event.h>
//for http
//#include <evhttp.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>
#include <signal.h>
#include <cJSON.h>

#define MYHTTPD_SIGNATURE   "MoCarHttpd v0.1"

//处理模块
void httpd_handler(struct evhttp_request *req, void *arg) {
    char output[2048] = "\0";
    char tmp[1024];

    //获取客户端请求的URI(使用evhttp_request_uri或直接req->uri)
    const char *uri;
    uri = evhttp_request_uri(req);
#if 0
    sprintf(tmp, "uri=%s\n", uri);//  /data?cmd=new...
    strcat(output, tmp);
#endif

    sprintf(tmp, "uri=%s\n", req->uri);
    strcat(output, tmp);

    //decoded uri
    char *decoded_uri;
    decoded_uri = evhttp_decode_uri(uri);
    sprintf(tmp, "decoded_uri=%s\n", decoded_uri);// /data?cmd= newFile ...
    strcat(output, tmp);

    //http://127.0.0.1:8080/data?cmd=newFile&fromId=0&count=8

    //解析URI的参数(即GET方法的参数)
    struct evkeyvalq params;//key ---value, key2--- value2//  cmd --- newfile  fromId == 0
    //将URL数据封装成key-value格式,q=value1, s=value2
    evhttp_parse_query(decoded_uri, &params);

    //得到q所对应的value
    sprintf(tmp, "username=%s\n", evhttp_find_header(&params, "username"));
    strcat(output, tmp);
    //得到s所对应的value
    sprintf(tmp, "passwd=%s\n", evhttp_find_header(&params, "passwd"));
    strcat(output, tmp);

    free(decoded_uri);

    //获取POST方法的数据
    char *post_data = (char *) EVBUFFER_DATA(req->input_buffer);
    sprintf(tmp, "post_data=%s\n", post_data);
    strcat(output, tmp);


    /*
       具体的：可以根据GET/POST的参数执行相应操作，然后将结果输出
       ...
     */


    /* 输出到客户端 */

    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    //输出的内容
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works!\n%s\n", output);

    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", buf);

    evbuffer_free(buf);

}


//处理login登陆模块
void login_handler(struct evhttp_request *req, void *arg) 
{
    printf("get connection!\n");

    //获取客户端请求的URI(使用evhttp_request_uri或直接req->uri)
    const char *uri;
    uri = evhttp_request_uri(req);
    printf("[uri]=%s\n", uri);


    //获取POST方法的数据
    size_t post_size = EVBUFFER_LENGTH(req->input_buffer);
    char *request_data = (char *) EVBUFFER_DATA(req->input_buffer);
    char request_data_buf[4096] = {0};
    memcpy(request_data_buf, request_data, post_size);
    printf("[post_data]=\n%s\n", request_data_buf);


    /*
       具体的：可以根据GET/POST的参数执行相应操作，然后将结果输出
       ...
     */

    //unpack json
    cJSON* root = cJSON_Parse(request_data_buf);
    cJSON* username = cJSON_GetObjectItem(root, "username");
    cJSON* password = cJSON_GetObjectItem(root, "password");

    printf("username = %s\n", username->valuestring);
    printf("password = %s\n", password->valuestring);

    cJSON_Delete(root);


    //packet json
    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "result", "ok");
    cJSON_AddStringToObject(root, "sessionid", "xxxxxxxx");

    char *response_data = cJSON_Print(root);
    cJSON_Delete(root);


    /* 输出到客户端 */
    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");

    //输出的内容
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s", response_data);

    //将封装好的evbuffer 发送给客户端
    evhttp_send_reply(req, HTTP_OK, "OK", buf);

    evbuffer_free(buf);

    printf("[response]:\n");
    printf("%s\n", response_data);

    free(response_data);

}

void show_help() {
    char *help = "http://localhost:8080\n"
        "-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
        "-p <num>     port number to listen on, default is 1984\n"
        "-d           run as a deamon\n"
        "-t <second>  timeout for a http request, default is 120 seconds\n"
        "-h           print this help and exit\n"
        "\n";
    fprintf(stderr,"%s",help);
}

//当向进程发出SIGTERM/SIGHUP/SIGINT/SIGQUIT的时候，终止event的事件侦听循环
void signal_handler(int sig) {
    switch (sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGINT:
            event_loopbreak();  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
            break;
    }
}

int main(int argc, char *argv[]) {
    //自定义信号处理函数
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    //默认参数
    char *httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 8080;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120; //in seconds

    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:h")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 'p' :
                httpd_option_port = atoi(optarg);
                break;
            case 'd' :
                httpd_option_daemon = 1;
                break;
            case 't' :
                httpd_option_timeout = atoi(optarg);
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }

    //判断是否设置了-d，以daemon运行
    if (httpd_option_daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            //生成子进程成功，退出父进程
            exit(EXIT_SUCCESS);
        }
    }


    /* 使用libevent创建HTTP Server */

    //初始化event API
    event_init();

    //创建一个http server
    struct evhttp *httpd;

    httpd = evhttp_start(httpd_option_listen, httpd_option_port);

    evhttp_set_timeout(httpd, httpd_option_timeout);

    //指定generic callback
    // evhttp_set_gencb(httpd, httpd_handler, NULL);
    //也可以为特定的URI指定callback
    evhttp_set_cb(httpd, "/", httpd_handler, NULL);
    evhttp_set_cb(httpd, "/login", login_handler,NULL);
#if 0
    evhttp_set_cb(httpd, "/upload", dsad)
        evhttp_set_cb(httpd, "/data", )
        evhttp_set_cb(httpd, "/reg", )
#endif

        //循环处理events
        event_dispatch();

    evhttp_free(httpd);

    return 0;
}
