#include "ml/xml.h"

#include "ml/common.h"

static const char *find_tag(const char *xml, const char *tag, int closing, int *len) {
    char open[96];
    const char *p;
    if (!xml || !tag) {
        return NULL;
    }
    snprintf(open, sizeof(open), closing ? "</%s>" : "<%s", tag);
    p = strstr(xml, open);
    if (!p) {
        return NULL;
    }
    if (closing) {
        if (len) {
            *len = (int)strlen(open);
        }
        return p;
    }
    p += strlen(open);
    while (*p && *p != '>') {
        p++;
    }
    if (*p != '>') {
        return NULL;
    }
    return p + 1;
}

int ml_xml_status_ok(const char *xml) {
    int st = 0;
    if (!xml) {
        return 0;
    }
    if (ml_xml_get_int(xml, "status_code", &st) != ML_OK) {
        /* Sunshine also uses attribute status_code="200" */
        const char *a = strstr(xml, "status_code=\"");
        if (a) {
            st = atoi(a + 13);
        }
    }
    return st == 200;
}

int ml_xml_get_str(const char *xml, const char *tag, char *out, int out_sz) {
    const char *s, *e;
    int n;
    if (!out || out_sz <= 0) {
        return ML_ERR;
    }
    out[0] = 0;
    s = find_tag(xml, tag, 0, NULL);
    if (!s) {
        return ML_ERR_PARSE;
    }
    e = find_tag(s, tag, 1, NULL);
    if (!e) {
        return ML_ERR_PARSE;
    }
    n = (int)(e - s);
    if (n >= out_sz) {
        n = out_sz - 1;
    }
    memcpy(out, s, (size_t)n);
    out[n] = 0;
    return ML_OK;
}

int ml_xml_get_int(const char *xml, const char *tag, int *out) {
    char buf[64];
    if (ml_xml_get_str(xml, tag, buf, sizeof(buf)) != ML_OK) {
        return ML_ERR_PARSE;
    }
    if (out) {
        *out = atoi(buf);
    }
    return ML_OK;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

int ml_xml_get_hex(const char *xml, const char *tag, unsigned char *out, int out_sz, int *out_len) {
    char *hex;
    char tmp[8192];
    int i, n, hi, lo;
    if (ml_xml_get_str(xml, tag, tmp, sizeof(tmp)) != ML_OK) {
        return ML_ERR_PARSE;
    }
    hex = tmp;
    n = (int)strlen(hex);
    if (n % 2) {
        return ML_ERR_PARSE;
    }
    if (n / 2 > out_sz) {
        return ML_ERR_MEM;
    }
    for (i = 0; i < n; i += 2) {
        hi = hexval(hex[i]);
        lo = hexval(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            return ML_ERR_PARSE;
        }
        out[i / 2] = (unsigned char)((hi << 4) | lo);
    }
    if (out_len) {
        *out_len = n / 2;
    }
    return ML_OK;
}
