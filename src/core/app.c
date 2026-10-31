#include "ml/app.h"

#include "ml/common.h"
#include "ml/gs.h"
#include "ml/input.h"
#include "ml/log.h"
#include "ml/rtsp.h"
#include "ml/stream.h"
#include "ml/ui.h"

enum { ST_SETUP, ST_CONNECTING, ST_MENU, ST_STREAM, ST_ERROR };

static void ip_from_octets(char *out, int out_sz, const int o[4]) {
    snprintf(out, (size_t)out_sz, "%d.%d.%d.%d", o[0], o[1], o[2], o[3]);
}

static void octets_from_ip(const char *ip, int o[4]) {
    o[0] = 192;
    o[1] = 168;
    o[2] = 1;
    o[3] = 10;
    if (ip && ip[0]) {
        sscanf(ip, "%d.%d.%d.%d", &o[0], &o[1], &o[2], &o[3]);
    }
    for (int i = 0; i < 4; i++) {
        if (o[i] < 0) {
            o[i] = 0;
        }
        if (o[i] > 255) {
            o[i] = 255;
        }
    }
}

int ml_app_run(const MlConfig *in_cfg) {
    MlConfig cfg = *in_cfg;
    MlPad pad, prev;
    MlServerInfo info;
    MlApp apps[24];
    MlStreamStats st;
    int octets[4];
    int cursor = 3;
    int state = ST_SETUP;
    int app_count = 0;
    int app_sel = 0;
    int running = 1;
    int rc;
    char line[160];
    char err[160];
    unsigned char rikey[16];
    int i;
    int wait_frames = 0;

    memset(&pad, 0, sizeof(pad));
    memset(&prev, 0, sizeof(prev));
    memset(&info, 0, sizeof(info));
    memset(&st, 0, sizeof(st));
    octets_from_ip(cfg.host, octets);
    ip_from_octets(cfg.host, sizeof(cfg.host), octets);
    for (i = 0; i < 16; i++) {
        rikey[i] = (unsigned char)(0xA5 ^ i);
    }

    if (ml_ui_init(1920, 1080) != ML_OK) {
        ml_log("ui init failed");
        return ML_ERR;
    }
    ml_input_init();
    ml_log_init(cfg.debug_host[0] ? cfg.debug_host : NULL, 9999);

    while (running) {
        int pressed;
        ml_input_poll(&pad);
        pressed = pad.buttons & ~prev.buttons;

        if (state == ST_SETUP) {
            if (pressed & 0x10) { /* left */
                cursor = (cursor + 3) % 4;
            }
            if (pressed & 0x20) { /* right */
                cursor = (cursor + 1) % 4;
            }
            if (pressed & 0x40) { /* up */
                octets[cursor] = (octets[cursor] + 1) % 256;
            }
            if (pressed & 0x80) { /* down */
                octets[cursor] = (octets[cursor] + 255) % 256;
            }
            if (pad.cross && !prev.cross) {
                ip_from_octets(cfg.host, sizeof(cfg.host), octets);
                state = ST_CONNECTING;
            }
            if (pad.options && !prev.options) {
                running = 0;
            }
        } else if (state == ST_MENU) {
            if (pressed & 0x40 && app_count) {
                app_sel = (app_sel + app_count - 1) % app_count;
            }
            if (pressed & 0x80 && app_count) {
                app_sel = (app_sel + 1) % app_count;
            }
            if (pad.cross && !prev.cross) {
                int app_id = cfg.app_id;
                if (app_count > 0) {
                    app_id = apps[app_sel].id;
                }
                rc = ml_gs_launch(&cfg, app_id ? app_id : 0, rikey, 1);
                ml_log("launch rc=%d app=%d", rc, app_id);
                ml_stream_open(ML_VIDEO_PORT);
                rc = ml_rtsp_play(cfg.host, ML_RTSP_PORT, cfg.width, cfg.height, cfg.fps, cfg.bitrate_kbps,
                                  line, (int)sizeof(line));
                ml_log("rtsp rc=%d session=%s", rc, line);
                state = ST_STREAM;
                wait_frames = 0;
            }
            if (pad.circle && !prev.circle) {
                state = ST_SETUP;
            }
            if (pad.options && !prev.options) {
                running = 0;
            }
        } else if (state == ST_STREAM) {
            ml_stream_poll(8, &st);
            wait_frames++;
            if (pad.options && pad.touchpad) {
                ml_gs_quit(&cfg);
                ml_stream_close();
                state = ST_MENU;
            }
            if (pad.circle && !prev.circle) {
                ml_gs_quit(&cfg);
                ml_stream_close();
                state = ST_MENU;
            }
        } else if (state == ST_CONNECTING) {
            memset(&info, 0, sizeof(info));
            rc = ml_gs_serverinfo(&cfg, 0, &info, NULL);
            if (rc != ML_OK) {
                snprintf(err, sizeof(err), "HTTP failed (%d). Is Sunshine running?", rc);
                state = ST_ERROR;
            } else {
                app_count = 0;
                ml_gs_applist(&cfg, apps, 24, &app_count);
                state = ST_MENU;
            }
        } else if (state == ST_ERROR) {
            if (pad.circle && !prev.circle) {
                state = ST_SETUP;
            }
            if (pad.options && !prev.options) {
                running = 0;
            }
        }

        ml_ui_clear(0x101418);
        ml_ui_text(64, 48, 0x7EC8E3, "Moonlight PS4  " ML_VERSION);
        ml_ui_text(64, 80, 0x8899AA, "Sunshine / GameStream client for jailbroken PS4");

        if (state == ST_SETUP) {
            ml_ui_text(64, 160, 0xFFFFFF, "Sunshine host IP  (D-Pad edit, X connect, OPTIONS quit)");
            for (i = 0; i < 4; i++) {
                snprintf(line, sizeof(line), "%03d", octets[i]);
                ml_ui_text(64 + i * 96, 210, i == cursor ? 0xFFE08A : 0xEEEEEE, line);
                if (i < 3) {
                    ml_ui_text(64 + i * 96 + 56, 210, 0x667788, ".");
                }
            }
            ml_ui_text(64, 300, 0x8899AA, "Or put host=x.x.x.x in /data/moonlight/moonlight.ini");
            ml_ui_text(64, 330, 0x8899AA, "First pair from another Moonlight client, or HTTP-only serverinfo.");
        } else if (state == ST_CONNECTING) {
            ml_ui_text(64, 180, 0xFFFFFF, "Connecting to Sunshine...");
        } else if (state == ST_MENU) {
            snprintf(line, sizeof(line), "Host: %s   paired=%d   https=%d", cfg.host, info.paired, info.https_port);
            ml_ui_text(64, 150, 0xFFFFFF, line);
            snprintf(line, sizeof(line), "Server: %s  GFE/Sun %s", info.hostname[0] ? info.hostname : "?",
                     info.gfeversion[0] ? info.gfeversion : info.appversion);
            ml_ui_text(64, 180, 0xAABBCC, line);
            ml_ui_text(64, 230, 0x7EC8E3, "Apps  (Up/Down, X start, O back)");
            if (app_count <= 0) {
                ml_ui_text(80, 270, 0xFFE08A, "No applist (HTTPS pairing required). Will launch Desktop id=0.");
            }
            for (i = 0; i < app_count && i < 12; i++) {
                snprintf(line, sizeof(line), "%s  [%d]", apps[i].name, apps[i].id);
                ml_ui_text(80, 270 + i * 28, i == app_sel ? 0xFFE08A : 0xDDDDDD, line);
            }
        } else if (state == ST_STREAM) {
            ml_ui_text(64, 160, 0x7EC8E3, "Streaming  (O = stop, OPTIONS+Touchpad = quit session)");
            snprintf(line, sizeof(line), "RTP pkts %lu  bytes %lu  NAL %lu  last type %d  drops %lu", st.packets,
                     st.bytes, st.nalus, st.last_nal_type, st.fu_drops);
            ml_ui_text(64, 210, 0xFFFFFF, line);
            ml_ui_text(64, 250, 0x8899AA, "H.264 RTP is received. Hardware decode lands in a later build.");
            if (st.packets == 0 && wait_frames > 120) {
                ml_ui_text(64, 290, 0xFF8080, "No video packets yet. Check RTSP 48010 / UDP 47998 and pairing.");
            }
        } else if (state == ST_ERROR) {
            ml_ui_text(64, 200, 0xFF8080, err);
            ml_ui_text(64, 250, 0xFFFFFF, "O = back");
        }

        ml_ui_present();
        prev = pad;
    }

    ml_stream_close();
    ml_input_shutdown();
    ml_ui_shutdown();
    return ML_OK;
}
