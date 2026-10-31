#include "ml/rtsp.h"

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

static int rtsp_exchange(int fd, const char *req, char *resp, int resp_sz) {
    int n, got = 0;
    if (send(fd, req, (int)strlen(req), 0) < 0) {
        return ML_ERR_NET;
    }
    while (got + 1 < resp_sz) {
        n = (int)recv(fd, resp + got, (size_t)(resp_sz - 1 - got), 0);
        if (n <= 0) {
            break;
        }
        got += n;
        resp[got] = 0;
        if (strstr(resp, "\r\n\r\n")) {
            break;
        }
    }
    return got > 0 ? ML_OK : ML_ERR_NET;
}

int ml_rtsp_play(const char *host, int port, int width, int height, int fps, int bitrate_kbps, char *session_id,
                 int session_sz) {
    int fd;
    struct sockaddr_in addr;
    char req[1024];
    char resp[2048];
    int seq = 1;
    int rc;

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
             "OPTIONS rtsp://%s:%d RTSP/1.0\r\nCSeq: %d\r\nX-GS-ClientVersion: 14\r\n\r\n", host, port, seq++);
    if (rtsp_exchange(fd, req, resp, sizeof(resp)) != ML_OK) {
        close(fd);
        return ML_ERR_NET;
    }

    snprintf(req, sizeof(req),
             "DESCRIBE rtsp://%s:%d RTSP/1.0\r\nCSeq: %d\r\nAccept: application/sdp\r\n"
             "x-ml-general.featureFlags: 67\r\n\r\n",
             host, port, seq++);
    rtsp_exchange(fd, req, resp, sizeof(resp));

    snprintf(req, sizeof(req),
             "SETUP rtsp://%s:%d/video RTSP/1.0\r\nCSeq: %d\r\nTransport: unicast;client_port=%d-%d\r\n"
             "If-Modified-Since: Thu, 01 Jan 1970 00:00:00 GMT\r\n\r\n",
             host, port, seq++, ML_VIDEO_PORT, ML_VIDEO_PORT + 1);
    rtsp_exchange(fd, req, resp, sizeof(resp));
    if (session_id && session_sz > 0) {
        const char *s = strstr(resp, "Session:");
        session_id[0] = 0;
        if (s) {
            int i = 0;
            s += 8;
            while (*s == ' ') {
                s++;
            }
            while (*s && *s != '\r' && *s != ';' && i + 1 < session_sz) {
                session_id[i++] = *s++;
            }
            session_id[i] = 0;
        }
    }

    snprintf(req, sizeof(req),
             "SETUP rtsp://%s:%d/audio RTSP/1.0\r\nCSeq: %d\r\nTransport: unicast;client_port=%d-%d\r\n"
             "Session: %s\r\n\r\n",
             host, port, seq++, ML_AUDIO_PORT, ML_AUDIO_PORT + 1, session_id ? session_id : "0");
    rtsp_exchange(fd, req, resp, sizeof(resp));

    snprintf(req, sizeof(req),
             "SETUP rtsp://%s:%d/control RTSP/1.0\r\nCSeq: %d\r\nTransport: unicast;client_port=%d-%d\r\n"
             "Session: %s\r\n\r\n",
             host, port, seq++, ML_CTRL_PORT, ML_CTRL_PORT + 1, session_id ? session_id : "0");
    rtsp_exchange(fd, req, resp, sizeof(resp));

    snprintf(req, sizeof(req),
             "ANNOUNCE rtsp://%s:%d RTSP/1.0\r\nCSeq: %d\r\nSession: %s\r\nContent-Type: application/sdp\r\n"
             "Content-Length: 0\r\n\r\n",
             host, port, seq++, session_id ? session_id : "0");
    rtsp_exchange(fd, req, resp, sizeof(resp));

    snprintf(req, sizeof(req),
             "PLAY rtsp://%s:%d RTSP/1.0\r\nCSeq: %d\r\nSession: %s\r\n"
             "x-ml-video.clientViewportWd: %d\r\n"
             "x-ml-video.clientViewportHt: %d\r\n"
             "x-ml-video.fps: %d\r\n"
             "x-nl-video.configuredBitrateKbps: %d\r\n\r\n",
             host, port, seq++, session_id ? session_id : "0", width, height, fps, bitrate_kbps);
    rc = rtsp_exchange(fd, req, resp, sizeof(resp));
    ml_log("RTSP PLAY: %.80s", resp);
    close(fd);
    return rc;
}
