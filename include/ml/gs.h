#ifndef ML_GS_H
#define ML_GS_H

#include "ml/config.h"

typedef struct MlServerInfo {
    char hostname[128];
    char appversion[32];
    char gfeversion[32];
    int https_port;
    int paired;
    int current_game;
    int max_luma_pixels;
    int server_codec_mode;
} MlServerInfo;

typedef struct MlApp {
    char name[96];
    int id;
} MlApp;

int ml_gs_serverinfo(const MlConfig *cfg, int tls, MlServerInfo *out, char **raw_xml);
int ml_gs_applist(const MlConfig *cfg, MlApp *apps, int max_apps, int *count);
int ml_gs_launch(const MlConfig *cfg, int app_id, const unsigned char rikey[16], int rikeyid);
int ml_gs_quit(const MlConfig *cfg);
int ml_gs_pair(const MlConfig *cfg, const char *pin, char *err, int err_sz);
void ml_gs_make_uuid(char *out, int out_sz);
void ml_gs_url_encode(const char *in, char *out, int out_sz);

#endif
