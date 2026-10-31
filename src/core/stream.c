#include "ml/stream.h"

#include "ml/common.h"
#include "ml/rtp.h"

#ifdef ML_PLATFORM_ORBIS
#include <net.h>
#include <sys/select.h>
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
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#endif

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

static int g_fd = -1;
static MlStreamStats g_stats;
static unsigned char g_nal[256 * 1024];
static int g_nal_len;

int ml_stream_open(int port) {
    struct sockaddr_in addr;
    int one = 1;
    ml_stream_close();
    memset(&g_stats, 0, sizeof(g_stats));
    g_nal_len = 0;
    g_fd = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (g_fd < 0) {
        return ML_ERR_NET;
    }
    setsockopt(g_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(g_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(g_fd);
        g_fd = -1;
        return ML_ERR_NET;
    }
    return ML_OK;
}

void ml_stream_close(void) {
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
    }
}

int ml_stream_poll(int timeout_ms, MlStreamStats *stats) {
    unsigned char pkt[2048];
    fd_set rfds;
    struct timeval tv;
    int n;
    MlRtpHeader hdr;
    int complete = 0;
    int nal_len = 0;

    if (g_fd < 0) {
        return ML_ERR;
    }
    FD_ZERO(&rfds);
    FD_SET(g_fd, &rfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    n = select(g_fd + 1, &rfds, NULL, NULL, &tv);
    if (n <= 0) {
        if (stats) {
            *stats = g_stats;
        }
        return n == 0 ? ML_OK : ML_ERR_NET;
    }
    n = (int)recv(g_fd, (char *)pkt, sizeof(pkt), 0);
    if (n <= 0) {
        return ML_ERR_NET;
    }
    g_stats.packets++;
    g_stats.bytes += (unsigned long)n;
    if (ml_rtp_parse(pkt, n, &hdr) != ML_OK) {
        if (stats) {
            *stats = g_stats;
        }
        return ML_OK;
    }
    nal_len = g_nal_len;
    if (ml_h264_depay(pkt + hdr.header_len, n - hdr.header_len, g_nal, (int)sizeof(g_nal), &nal_len,
                      &complete) != ML_OK) {
        g_stats.fu_drops++;
        g_nal_len = 0;
    } else {
        g_nal_len = nal_len;
        if (complete && nal_len > 4) {
            g_stats.nalus++;
            g_stats.last_nal_type = g_nal[4] & 0x1f;
            g_nal_len = 0;
        }
    }
    if (stats) {
        *stats = g_stats;
    }
    ML_UNUSED(MSG_DONTWAIT);
    return ML_OK;
}
