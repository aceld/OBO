/**
 * @file https-server.c
 * @brief  
 * @author liu_danbing
 * @version 1.0
 * @date 2016-10-26
 */

#include "https-common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <event2/bufferevent.h>
#include <event2/bufferevent_ssl.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#include "busi_cb.h"
#include "config.h"
#include "util.h"



#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif

unsigned short serverPort = COMMON_HTTPS_PORT;

/* Instead of casting between these types, create a union with all of them,
 * to avoid -Wstrict-aliasing warnings. */
typedef union
{ 
    struct sockaddr_storage ss;
    struct sockaddr sa;
    struct sockaddr_in in;
    struct sockaddr_in6 i6;
} sock_hop;



/**
 * This callback is responsible for creating a new SSL connection
 * and wrapping it in an OpenSSL bufferevent.  This is the way
 * we implement an https server instead of a plain old http server.
 */
static struct bufferevent* bevcb (struct event_base *base, void *arg)
{ 
    struct bufferevent* r;
    SSL_CTX *ctx = (SSL_CTX *) arg;

    r = bufferevent_openssl_socket_new (base,
            -1,
            SSL_new (ctx),
            BUFFEREVENT_SSL_ACCEPTING,
            BEV_OPT_CLOSE_ON_FREE);
    return r;
}

static void server_setup_certs (SSL_CTX *ctx,
        const char *certificate_chain,
        const char *private_key)
{ 
    info_report ("Loading certificate chain from '%s'\n"
            "and private key from '%s'\n",
            certificate_chain, private_key);

    if (1 != SSL_CTX_use_certificate_chain_file (ctx, certificate_chain))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_certificate_chain_file");

    if (1 != SSL_CTX_use_PrivateKey_file (ctx, private_key, SSL_FILETYPE_PEM))
        die_most_horribly_from_openssl_error ("SSL_CTX_use_PrivateKey_file");

    if (1 != SSL_CTX_check_private_key (ctx))
        die_most_horribly_from_openssl_error ("SSL_CTX_check_private_key");
}

static int serve_some_http (void)
{ 
    struct event_base *base;
    struct evhttp *http;
    struct evhttp_bound_socket *handle;

    
    base = event_base_new ();
    if (! base)
    { 
        fprintf (stderr, "Couldn't create an event_base: exiting\n");
        return 1;
    }

    /* 创建一个 evhttp 句柄，去处理用户端的requests请求 */
    http = evhttp_new (base);
    if (! http)
    { fprintf (stderr, "couldn't create evhttp. Exiting.\n");
        return 1;
    }

    /* 创建SSL上下文环境 ，可以理解为 SSL句柄 */
    SSL_CTX *ctx = SSL_CTX_new (SSLv23_server_method ());
    SSL_CTX_set_options (ctx,
            SSL_OP_SINGLE_DH_USE |
            SSL_OP_SINGLE_ECDH_USE |
            SSL_OP_NO_SSLv2);

    /* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
     * We just hardcode a single curve which is reasonably decent.
     * See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */
    EC_KEY *ecdh = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);
    if (! ecdh)
        die_most_horribly_from_openssl_error ("EC_KEY_new_by_curve_name");
    if (1 != SSL_CTX_set_tmp_ecdh (ctx, ecdh))
        die_most_horribly_from_openssl_error ("SSL_CTX_set_tmp_ecdh");

    /* 选择服务器证书 和 服务器私钥. */
    const char *certificate_chain = "server-certificate-chain.pem";
    const char *private_key = "server-private-key.pem";
    /* 设置服务器证书 和 服务器私钥 到 
     OPENSSL ctx上下文句柄中 */
    server_setup_certs (ctx, certificate_chain, private_key);

    /* 
        使我们创建好的evhttp句柄 支持 SSL加密
        实际上，加密的动作和解密的动作都已经帮
        我们自动完成，我们拿到的数据就已经解密之后的
    */
    evhttp_set_bevcb (http, bevcb, ctx);

    /* 设置http回调函数 */
    //默认回调
    //evhttp_set_gencb (http, send_document_cb, NULL);
    //专属uri路径回调
    evhttp_set_cb(http, "/persistent", persistent_store_cb, NULL);
    evhttp_set_cb(http, "/cache", cache_store_cb, NULL);

    /* 设置监听IP和端口 */
    handle = evhttp_bind_socket_with_handle (http, "0.0.0.0", serverPort);
    if (! handle)
    { 
        fprintf (stderr, "couldn't bind to port %d. Exiting.\n",(int) serverPort);
        return 1;
    }

    { 
        /* 打印收到的客户端链接信息  */
        sock_hop ss;
        evutil_socket_t fd;
        ev_socklen_t socklen = sizeof (ss);
        char addrbuf[128];
        void *inaddr;
        const char *addr;
        int got_port = -1;

        fd = evhttp_bound_socket_get_fd (handle);
        memset (&ss, 0, sizeof(ss));

        if (getsockname (fd, &ss.sa, &socklen))
        { 
            perror ("getsockname() failed");
            return 1;
        }
        if (ss.ss.ss_family == AF_INET)
        { 
            got_port = ntohs (ss.in.sin_port);
            inaddr = &ss.in.sin_addr;
        }
        else if (ss.ss.ss_family == AF_INET6)
        { 
            got_port = ntohs (ss.i6.sin6_port);
            inaddr = &ss.i6.sin6_addr;
        }
        else
        { 
            fprintf (stderr, "Weird address family %d\n", ss.ss.ss_family);
            return 1;
        }
        addr = evutil_inet_ntop (ss.ss.ss_family, inaddr, addrbuf,
                sizeof (addrbuf));
        if (addr)
            printf ("Listening on %s:%d\n", addr, got_port);
        else
        { 
            fprintf (stderr, "evutil_inet_ntop failed\n");
            return 1;
        }
    }

    /* 开始阻塞监听 (永久执行) */
    event_base_dispatch (base);


    return 0;
}

int main (int argc, char **argv)
{ 
    /*OpenSSL 初始化 */
    common_setup ();              

    if (argc > 1) {
        char *end_ptr;
        long lp = strtol(argv[1], &end_ptr, 0);
        if (*end_ptr) {
            fprintf(stderr, "Invalid integer\n");
            return -1;
        }
        if (lp <= 0) {
            fprintf(stderr, "Port must be positive\n");
            return -1;
        }
        if (lp >= USHRT_MAX) {
            fprintf(stderr, "Port must fit 16-bit range\n");
            return -1;
        }

        serverPort = (unsigned short)lp;
    }
    config_init();

    /* now run http server (never returns) */
    return serve_some_http ();
}
