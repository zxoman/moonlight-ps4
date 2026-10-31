#include "ml/rtp.h"

#include "ml/common.h"

int ml_rtp_parse(const unsigned char *pkt, int len, MlRtpHeader *hdr) {
    int csrc, ext;
    if (!pkt || !hdr || len < 12) {
        return ML_ERR;
    }
    if ((pkt[0] >> 6) != 2) {
        return ML_ERR_PARSE;
    }
    csrc = pkt[0] & 0x0f;
    ext = (pkt[0] >> 4) & 1;
    hdr->marker = (pkt[1] >> 7) & 1;
    hdr->payload_type = pkt[1] & 0x7f;
    hdr->seq = (uint16_t)((pkt[2] << 8) | pkt[3]);
    hdr->timestamp = ((uint32_t)pkt[4] << 24) | ((uint32_t)pkt[5] << 16) | ((uint32_t)pkt[6] << 8) | pkt[7];
    hdr->ssrc = ((uint32_t)pkt[8] << 24) | ((uint32_t)pkt[9] << 16) | ((uint32_t)pkt[10] << 8) | pkt[11];
    hdr->header_len = 12 + csrc * 4;
    if (ext) {
        int ext_len;
        if (len < hdr->header_len + 4) {
            return ML_ERR_PARSE;
        }
        ext_len = (pkt[hdr->header_len + 2] << 8) | pkt[hdr->header_len + 3];
        hdr->header_len += 4 + ext_len * 4;
    }
    if (hdr->header_len > len) {
        return ML_ERR_PARSE;
    }
    return ML_OK;
}

int ml_h264_depay(const unsigned char *payload, int len, unsigned char *nal, int nal_max, int *nal_len,
                  int *fu_complete) {
    unsigned char nal_type;
    if (fu_complete) {
        *fu_complete = 0;
    }
    if (!payload || len < 1 || !nal || !nal_len) {
        return ML_ERR;
    }
    nal_type = payload[0] & 0x1f;
    if (nal_type >= 1 && nal_type <= 23) {
        if (len + 4 > nal_max) {
            return ML_ERR_MEM;
        }
        nal[0] = 0;
        nal[1] = 0;
        nal[2] = 0;
        nal[3] = 1;
        memcpy(nal + 4, payload, (size_t)len);
        *nal_len = len + 4;
        if (fu_complete) {
            *fu_complete = 1;
        }
        return ML_OK;
    }
    if (nal_type == 28) { /* FU-A */
        unsigned char fu_header, start, end, type, nri;
        if (len < 2) {
            return ML_ERR_PARSE;
        }
        fu_header = payload[1];
        start = (fu_header >> 7) & 1;
        end = (fu_header >> 6) & 1;
        type = fu_header & 0x1f;
        nri = payload[0] & 0x60;
        if (start) {
            if (len - 2 + 5 > nal_max) {
                return ML_ERR_MEM;
            }
            nal[0] = 0;
            nal[1] = 0;
            nal[2] = 0;
            nal[3] = 1;
            nal[4] = nri | type;
            memcpy(nal + 5, payload + 2, (size_t)len - 2);
            *nal_len = len + 3;
        } else {
            if (*nal_len + (len - 2) > nal_max) {
                return ML_ERR_MEM;
            }
            memcpy(nal + *nal_len, payload + 2, (size_t)len - 2);
            *nal_len += len - 2;
        }
        if (fu_complete) {
            *fu_complete = end ? 1 : 0;
        }
        return ML_OK;
    }
    return ML_ERR_UNSUPPORTED;
}
