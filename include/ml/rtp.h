#ifndef ML_RTP_H
#define ML_RTP_H

#include <stdint.h>

typedef struct MlRtpHeader {
    int payload_type;
    int marker;
    uint16_t seq;
    uint32_t timestamp;
    uint32_t ssrc;
    int header_len;
} MlRtpHeader;

int ml_rtp_parse(const unsigned char *pkt, int len, MlRtpHeader *hdr);
int ml_h264_depay(const unsigned char *payload, int len, unsigned char *nal, int nal_max, int *nal_len,
                  int *fu_complete);

#endif
