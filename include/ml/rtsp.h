#ifndef ML_RTSP_H
#define ML_RTSP_H

int ml_rtsp_play(const char *host, int port, int width, int height, int fps, int bitrate_kbps,
                 char *session_id, int session_sz);

#endif
