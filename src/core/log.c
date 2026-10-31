#include "ml/log.h"

#include "ml/common.h"

#include <stdarg.h>

#ifdef ML_PLATFORM_ORBIS
#include <net.h>
#include <sys/socket.h>
#else
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#endif

static int g_sock = -1;
static struct sockaddr_in g_addr;
static int g_udp;

void ml_log_init(const char *udp_host, int udp_port) {
    g_udp = 0;
    g_sock = -1;
    if (!udp_host || !udp_host[0] || udp_port <= 0) {
        return;
    }
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    g_sock = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sock < 0) {
        return;
    }
    memset(&g_addr, 0, sizeof(g_addr));
    g_addr.sin_family = AF_INET;
    g_addr.sin_port = htons((unsigned short)udp_port);
#ifdef ML_PLATFORM_ORBIS
    if (sceNetInetPton(AF_INET, udp_host, &g_addr.sin_addr) != 1) {
        return;
    }
#else
    if (inet_pton(AF_INET, udp_host, &g_addr.sin_addr) != 1) {
        return;
    }
#endif
    g_udp = 1;
}

void ml_log(const char *fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    fputs(buf, stdout);
    fputc('\n', stdout);
    fflush(stdout);
    if (g_udp && g_sock >= 0) {
        sendto(g_sock, buf, (int)strlen(buf), 0, (struct sockaddr *)&g_addr, sizeof(g_addr));
    }
}
