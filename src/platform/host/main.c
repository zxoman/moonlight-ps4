#include "ml/app.h"
#include "ml/common.h"
#include "ml/config.h"
#include "ml/gs.h"
#include "ml/http.h"
#include "ml/log.h"
#include "ml/rtp.h"
#include "ml/xml.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#endif

#include <stdio.h>

static int cmd_serverinfo(const char *host) {
    MlConfig cfg;
    MlServerInfo info;
    char *xml = NULL;
    int rc;
    ml_config_defaults(&cfg);
    snprintf(cfg.host, sizeof(cfg.host), "%s", host);
    rc = ml_gs_serverinfo(&cfg, 0, &info, &xml);
    if (rc != ML_OK) {
        fprintf(stderr, "serverinfo failed: %d\n", rc);
        return 1;
    }
    printf("hostname=%s paired=%d https=%d version=%s\n", info.hostname, info.paired, info.https_port,
           info.appversion);
    if (xml) {
        fwrite(xml, 1, strlen(xml), stdout);
        fputc('\n', stdout);
        free(xml);
    }
    return 0;
}

static void usage(const char *argv0) {
    fprintf(stderr,
            "moonlight-ps4 host tools %s\n"
            "  %s --help\n"
            "  %s --serverinfo <host-ip>\n"
            "  %s --ui   (framebuffer HUD on host; no real pad)\n",
            ML_VERSION, argv0, argv0, argv0);
}

int main(int argc, char **argv) {
    MlConfig cfg;
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    ml_config_defaults(&cfg);
    ml_config_load_file(&cfg, "moonlight.ini");
    ml_log_init(cfg.debug_host, 9999);

    if (argc >= 2 && !strcmp(argv[1], "--serverinfo") && argc >= 3) {
        return cmd_serverinfo(argv[2]);
    }
    if (argc >= 2 && !strcmp(argv[1], "--help")) {
        usage(argv[0]);
        return 0;
    }
    if (argc >= 2 && !strcmp(argv[1], "--ui")) {
        if (!cfg.host[0] && argc >= 3) {
            snprintf(cfg.host, sizeof(cfg.host), "%s", argv[2]);
        }
        return ml_app_run(&cfg) == ML_OK ? 0 : 1;
    }
    if (argc >= 2) {
        usage(argv[0]);
        return 1;
    }
    usage(argv[0]);
    return 0;
}
