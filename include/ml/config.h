#ifndef ML_CONFIG_H
#define ML_CONFIG_H

typedef struct MlConfig {
    char host[128];
    char unique_id[17];
    char debug_host[128];
    int width;
    int height;
    int fps;
    int bitrate_kbps;
    int app_id;
    int http_port;
    int https_port;
} MlConfig;

void ml_config_defaults(MlConfig *c);
int ml_config_load_text(MlConfig *c, const char *text);
int ml_config_load_file(MlConfig *c, const char *path);

#endif
