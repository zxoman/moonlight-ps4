#include "ml/common.h"
#include "ml/gs.h"

#include <time.h>

void ml_gs_make_uuid(char *out, int out_sz) {
    unsigned int a, b, c, d;
    if (!out || out_sz < 37) {
        return;
    }
    srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)out);
    a = (unsigned)rand();
    b = (unsigned)rand();
    c = (unsigned)rand();
    d = (unsigned)rand();
    snprintf(out, (size_t)out_sz, "%08x-%04x-%04x-%04x-%04x%08x", a, b & 0xffff, (c & 0x0fff) | 0x4000,
             (d & 0x3fff) | 0x8000, (b >> 16) & 0xffff, c ^ d);
}

void ml_gs_url_encode(const char *in, char *out, int out_sz) {
    static const char *hex = "0123456789ABCDEF";
    int o = 0;
    if (!in || !out || out_sz <= 0) {
        return;
    }
    for (; *in && o + 4 < out_sz; in++) {
        unsigned char c = (unsigned char)*in;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' ||
            c == '.' || c == '~') {
            out[o++] = (char)c;
        } else {
            out[o++] = '%';
            out[o++] = hex[c >> 4];
            out[o++] = hex[c & 15];
        }
    }
    out[o] = 0;
}

void ml_bytes_to_hex(const unsigned char *in, int in_len, char *out, int out_sz) {
    static const char *hex = "0123456789ABCDEF";
    int i, o = 0;
    for (i = 0; i < in_len && o + 2 < out_sz; i++) {
        out[o++] = hex[in[i] >> 4];
        out[o++] = hex[in[i] & 15];
    }
    if (out_sz > 0) {
        out[o < out_sz ? o : out_sz - 1] = 0;
    }
}
