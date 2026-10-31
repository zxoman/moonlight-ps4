#ifdef ML_PLATFORM_ORBIS
#include <orbis/Net.h>
#include <orbis/Sysmodule.h>
#include <orbis/SystemService.h>
#include <orbis/UserService.h>
#endif

#include "ml/app.h"
#include "ml/common.h"
#include "ml/config.h"
#include "ml/log.h"

#include <stdio.h>

#ifdef ML_PLATFORM_ORBIS
static char g_netmem[1024 * 1024];

static int net_init(void) {
    OrbisNetInitParam p;
    sceSysmoduleLoadModuleInternal((int)0x8000001C);
    memset(&p, 0, sizeof(p));
    p.memory = g_netmem;
    p.memorySize = (int)sizeof(g_netmem);
    if (sceNetInit(&p) < 0) {
        return ML_ERR_NET;
    }
    sceNetPoolCreate("ml", 16 * 1024, 0);
    return ML_OK;
}
#endif

int main(void) {
    MlConfig cfg;
    ml_config_defaults(&cfg);
    if (ml_config_load_file(&cfg, "/data/moonlight/moonlight.ini") != ML_OK) {
        FILE *f = fopen("/data/moonlight/debug_host.txt", "rb");
        if (f) {
            char ip[128];
            memset(ip, 0, sizeof(ip));
            if (fgets(ip, (int)sizeof(ip), f)) {
                size_t n = strlen(ip);
                while (n && (ip[n - 1] == '\n' || ip[n - 1] == '\r' || ip[n - 1] == ' ')) {
                    ip[--n] = 0;
                }
                snprintf(cfg.host, sizeof(cfg.host), "%s", ip);
            }
            fclose(f);
        }
        ml_config_load_file(&cfg, "/app0/moonlight.ini");
        if (!cfg.host[0]) {
            /* keep empty; UI editor fills it */
        }
    }

#ifdef ML_PLATFORM_ORBIS
    sceUserServiceInitialize(NULL);
    sceSystemServiceHideSplashScreen();
    if (net_init() != ML_OK) {
        ml_log("net init failed");
    }
#endif
    return ml_app_run(&cfg) == ML_OK ? 0 : 1;
}
