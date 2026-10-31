#include "ml/config.h"

#include "ml/common.h"

void ml_config_defaults(MlConfig *c) {
    memset(c, 0, sizeof(*c));
    memcpy(c->unique_id, "0123456789ABCDEF", 16);
    c->width = 1920;
    c->height = 1080;
    c->fps = 60;
    c->bitrate_kbps = 15000;
    c->app_id = 0; /* desktop */
    c->http_port = ML_HTTP_PORT;
    c->https_port = ML_HTTPS_PORT;
}

static void trim(char *s) {
    char *e;
    while (*s == ' ' || *s == '\t' || *s == '\r') {
        memmove(s, s + 1, strlen(s));
    }
    e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n')) {
        *--e = 0;
    }
}

int ml_config_load_text(MlConfig *c, const char *text) {
    char line[256];
    const char *p = text;
    ml_config_defaults(c);
    while (*p) {
        const char *nl = strchr(p, '\n');
        size_t n = nl ? (size_t)(nl - p) : strlen(p);
        if (n >= sizeof(line)) {
            n = sizeof(line) - 1;
        }
        memcpy(line, p, n);
        line[n] = 0;
        p += n;
        if (*p == '\n') {
            p++;
        }
        trim(line);
        if (!line[0] || line[0] == '#' || line[0] == ';') {
            continue;
        }
        char *eq = strchr(line, '=');
        if (!eq) {
            continue;
        }
        *eq++ = 0;
        trim(line);
        trim(eq);
        if (!strcmp(line, "host")) {
            snprintf(c->host, sizeof(c->host), "%s", eq);
        } else if (!strcmp(line, "uniqueid") || !strcmp(line, "unique_id")) {
            snprintf(c->unique_id, sizeof(c->unique_id), "%s", eq);
        } else if (!strcmp(line, "debug_host")) {
            snprintf(c->debug_host, sizeof(c->debug_host), "%s", eq);
        } else if (!strcmp(line, "width")) {
            c->width = atoi(eq);
        } else if (!strcmp(line, "height")) {
            c->height = atoi(eq);
        } else if (!strcmp(line, "fps")) {
            c->fps = atoi(eq);
        } else if (!strcmp(line, "bitrate") || !strcmp(line, "bitrate_kbps")) {
            c->bitrate_kbps = atoi(eq);
        } else if (!strcmp(line, "appid") || !strcmp(line, "app_id")) {
            c->app_id = atoi(eq);
        } else if (!strcmp(line, "http_port")) {
            c->http_port = atoi(eq);
        } else if (!strcmp(line, "https_port")) {
            c->https_port = atoi(eq);
        }
    }
    if ((int)strlen(c->unique_id) != 16) {
        memcpy(c->unique_id, "0123456789ABCDEF", 16);
        c->unique_id[16] = 0;
    }
    if (c->width <= 0) {
        c->width = 1920;
    }
    if (c->height <= 0) {
        c->height = 1080;
    }
    if (c->fps <= 0) {
        c->fps = 60;
    }
    if (c->bitrate_kbps <= 0) {
        c->bitrate_kbps = 15000;
    }
    if (c->http_port <= 0) {
        c->http_port = ML_HTTP_PORT;
    }
    if (c->https_port <= 0) {
        c->https_port = ML_HTTPS_PORT;
    }
    return ML_OK;
}

int ml_config_load_file(MlConfig *c, const char *path) {
    FILE *f = fopen(path, "rb");
    char *buf;
    long sz;
    int rc;
    if (!f) {
        ml_config_defaults(c);
        return ML_ERR;
    }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0 || sz > 64 * 1024) {
        fclose(f);
        return ML_ERR;
    }
    buf = (char *)ml_xmalloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return ML_ERR_MEM;
    }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return ML_ERR;
    }
    buf[sz] = 0;
    fclose(f);
    rc = ml_config_load_text(c, buf);
    free(buf);
    return rc;
}
