#include "ml/http.h"

#include "ml/common.h"
#include "ml/log.h"

#ifdef ML_PLATFORM_ORBIS
#include <net.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#define close closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#endif

int ml_http_get(const char *host, int port, int tls, const char *path_and_query, char **body, int *body_len) {
    int fd;
    struct sockaddr_in addr;
    char req[1536];
    char *buf;
    int cap = 64 * 1024;
    int got = 0;
    char *hdr_end;
    int n;
    int content_len = -1;
    int status = 0;

    if (tls) {
        /* TLS (HTTPS 47984) needs Mbed TLS; callers fall back to HTTP. */
        return ML_ERR_UNSUPPORTED;
    }
    if (!host || !path_and_query || !body) {
        return ML_ERR;
    }
    *body = NULL;
    if (body_len) {
        *body_len = 0;
    }

    fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return ML_ERR_NET;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
#ifdef ML_PLATFORM_ORBIS
    if (sceNetInetPton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return ML_ERR_NET;
    }
#else
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return ML_ERR_NET;
    }
#endif
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return ML_ERR_NET;
    }

    snprintf(req, sizeof(req),
             "GET %s HTTP/1.0\r\nHost: %s:%d\r\nConnection: close\r\nUser-Agent: moonlight-ps4/%s\r\n\r\n",
             path_and_query, host, port, ML_VERSION);
    if (send(fd, req, (int)strlen(req), 0) < 0) {
        close(fd);
        return ML_ERR_NET;
    }

    buf = (char *)malloc((size_t)cap);
    if (!buf) {
        close(fd);
        return ML_ERR_MEM;
    }
    while (got + 1 < cap) {
        n = (int)recv(fd, buf + got, (size_t)(cap - 1 - got), 0);
        if (n <= 0) {
            break;
        }
        got += n;
        if (got > cap - 4096 && cap < 512 * 1024) {
            char *nb = (char *)realloc(buf, (size_t)cap * 2);
            if (!nb) {
                break;
            }
            buf = nb;
            cap *= 2;
        }
    }
    close(fd);
    buf[got] = 0;

    if (sscanf(buf, "HTTP/%*s %d", &status) != 1) {
        free(buf);
        return ML_ERR_PARSE;
    }
    hdr_end = strstr(buf, "\r\n\r\n");
    if (!hdr_end) {
        free(buf);
        return ML_ERR_PARSE;
    }
    {
        char *cl = buf;
        while (cl && cl < hdr_end) {
            if (!strncmp(cl, "Content-Length:", 15) || !strncmp(cl, "content-length:", 15)) {
                content_len = atoi(cl + 15);
                break;
            }
            cl = strstr(cl, "\r\n");
            if (!cl) {
                break;
            }
            cl += 2;
        }
    }
    hdr_end += 4;
    n = got - (int)(hdr_end - buf);
    if (content_len >= 0 && n > content_len) {
        n = content_len;
    }
    {
        char *out = (char *)malloc((size_t)n + 1);
        if (!out) {
            free(buf);
            return ML_ERR_MEM;
        }
        memcpy(out, hdr_end, (size_t)n);
        out[n] = 0;
        free(buf);
        *body = out;
        if (body_len) {
            *body_len = n;
        }
    }
    ML_UNUSED(status);
    return ML_OK;
}
