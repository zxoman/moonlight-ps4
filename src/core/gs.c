#include "ml/gs.h"

#include "ml/common.h"
#include "ml/http.h"
#include "ml/util.h"
#include "ml/xml.h"

static int gs_get(const MlConfig *cfg, int tls, const char *path, char **body) {
    int len = 0;
    int port = tls ? cfg->https_port : cfg->http_port;
    return ml_http_get(cfg->host, port, tls, path, body, &len);
}

int ml_gs_serverinfo(const MlConfig *cfg, int tls, MlServerInfo *out, char **raw_xml) {
    char uuid[48];
    char path[256];
    char *body = NULL;
    int rc;
    ml_gs_make_uuid(uuid, sizeof(uuid));
    snprintf(path, sizeof(path), "/serverinfo?uniqueid=%s&uuid=%s", cfg->unique_id, uuid);
    rc = gs_get(cfg, tls, path, &body);
    if (rc != ML_OK) {
        return rc;
    }
    if (raw_xml) {
        *raw_xml = body;
    }
    if (!ml_xml_status_ok(body) && strstr(body, "<root") == NULL) {
        if (!raw_xml) {
            free(body);
        }
        return ML_ERR_PARSE;
    }
    if (out) {
        memset(out, 0, sizeof(*out));
        ml_xml_get_str(body, "hostname", out->hostname, sizeof(out->hostname));
        ml_xml_get_str(body, "appversion", out->appversion, sizeof(out->appversion));
        ml_xml_get_str(body, "GfeVersion", out->gfeversion, sizeof(out->gfeversion));
        ml_xml_get_int(body, "HttpsPort", &out->https_port);
        ml_xml_get_int(body, "PairStatus", &out->paired);
        ml_xml_get_int(body, "currentgame", &out->current_game);
        ml_xml_get_int(body, "MaxLumaPixelsHEVC", &out->max_luma_pixels);
        ml_xml_get_int(body, "ServerCodecModeSupport", &out->server_codec_mode);
        if (out->https_port <= 0) {
            out->https_port = cfg->https_port;
        }
    }
    if (!raw_xml) {
        free(body);
    }
    return ML_OK;
}

int ml_gs_applist(const MlConfig *cfg, MlApp *apps, int max_apps, int *count) {
    char uuid[48];
    char path[256];
    char *body = NULL;
    const char *p;
    int n = 0;
    int rc;
    ml_gs_make_uuid(uuid, sizeof(uuid));
    snprintf(path, sizeof(path), "/applist?uniqueid=%s&uuid=%s", cfg->unique_id, uuid);
    rc = gs_get(cfg, 1, path, &body);
    if (rc != ML_OK) {
        rc = gs_get(cfg, 0, path, &body);
    }
    if (rc != ML_OK) {
        return rc;
    }
    p = body;
    while (n < max_apps) {
        const char *app = strstr(p, "<App");
        const char *end;
        char slice[512];
        int sl;
        if (!app) {
            break;
        }
        end = strstr(app, "</App>");
        if (!end) {
            break;
        }
        sl = (int)(end + 6 - app);
        if (sl >= (int)sizeof(slice)) {
            sl = (int)sizeof(slice) - 1;
        }
        memcpy(slice, app, (size_t)sl);
        slice[sl] = 0;
        memset(&apps[n], 0, sizeof(apps[n]));
        ml_xml_get_str(slice, "AppTitle", apps[n].name, sizeof(apps[n].name));
        ml_xml_get_int(slice, "ID", &apps[n].id);
        n++;
        p = end + 6;
    }
    if (count) {
        *count = n;
    }
    free(body);
    return ML_OK;
}

int ml_gs_launch(const MlConfig *cfg, int app_id, const unsigned char rikey[16], int rikeyid) {
    char uuid[48];
    char hex[33];
    char path[768];
    char *body = NULL;
    int rc;
    ml_gs_make_uuid(uuid, sizeof(uuid));
    ml_bytes_to_hex(rikey, 16, hex, sizeof(hex));
    snprintf(path, sizeof(path),
             "/launch?uniqueid=%s&uuid=%s&appid=%d&mode=%dx%dx%d&additionalStates=1&sops=1&rik="
             "1&rikey=%s&rikeyid=%d&localAudioPlayMode=0&surroundAudioInfo=51&remoteControllersBitmap=0&gcmap=0",
             cfg->unique_id, uuid, app_id, cfg->width, cfg->height, cfg->fps, hex, rikeyid);
    rc = gs_get(cfg, 1, path, &body);
    if (rc != ML_OK) {
        rc = gs_get(cfg, 0, path, &body);
    }
    if (rc != ML_OK) {
        return rc;
    }
    rc = ml_xml_status_ok(body) ? ML_OK : ML_ERR;
    free(body);
    return rc;
}

int ml_gs_quit(const MlConfig *cfg) {
    char uuid[48];
    char path[256];
    char *body = NULL;
    int rc;
    ml_gs_make_uuid(uuid, sizeof(uuid));
    snprintf(path, sizeof(path), "/cancel?uniqueid=%s&uuid=%s", cfg->unique_id, uuid);
    rc = gs_get(cfg, 1, path, &body);
    if (rc != ML_OK) {
        rc = gs_get(cfg, 0, path, &body);
    }
    if (body) {
        free(body);
    }
    return rc;
}

int ml_gs_pair(const MlConfig *cfg, const char *pin, char *err, int err_sz) {
    ML_UNUSED(cfg);
    ML_UNUSED(pin);
    if (err && err_sz > 0) {
        snprintf(err, (size_t)err_sz,
                 "Pairing needs Mbed TLS (AES/SHA/RSA). Run scripts/fetch_deps.sh and rebuild.");
    }
    return ML_ERR_UNSUPPORTED;
}
