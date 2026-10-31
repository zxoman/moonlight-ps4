#include "ml/ui.h"

#include "ml/common.h"
#include "font8.h"

#ifdef ML_PLATFORM_ORBIS
#include <orbis/VideoOut.h>
#include <orbis/Sysmodule.h>
#endif

#ifndef ML_PLATFORM_ORBIS
static unsigned int *g_fb;
static int g_w, g_h;
#else
static int g_handle = -1;
static void *g_addr;
static int g_w, g_h;
static unsigned int *g_fb;
#endif

int ml_ui_width(void) { return g_w; }
int ml_ui_height(void) { return g_h; }

#ifdef ML_PLATFORM_ORBIS
int ml_ui_init(int width, int height) {
    OrbisVideoOutBufferAttribute attr;
    int rc;
    g_w = width;
    g_h = height;
    g_handle = sceVideoOutOpen(ORBIS_VIDEO_USER_MAIN, ORBIS_VIDEO_OUT_BUS_MAIN, 0, NULL);
    if (g_handle < 0) {
        return ML_ERR;
    }
    /* 1920x1080 tile-unaware linear dump via malloc (works on many HEN setups for HUD). */
    g_fb = (unsigned int *)aligned_alloc(64, (size_t)width * (size_t)height * 4);
    if (!g_fb) {
        return ML_ERR_MEM;
    }
    g_addr = g_fb;
    memset(&attr, 0, sizeof(attr));
    sceVideoOutSetBufferAttribute(&attr, 0x80000000 /* PIXEL_A8R8G8B8 */, 0, 0, width, height, width);
    rc = sceVideoOutRegisterBuffers(g_handle, 0, &g_addr, 1, &attr);
    if (rc < 0) {
        /* still allow CPU-side HUD if flip fails */
    }
    return ML_OK;
}

void ml_ui_shutdown(void) {
    if (g_handle >= 0) {
        sceVideoOutClose(g_handle);
        g_handle = -1;
    }
}

void ml_ui_present(void) {
    if (g_handle >= 0) {
        sceVideoOutSubmitFlip(g_handle, 0, ORBIS_VIDEO_OUT_FLIP_VSYNC, 0);
        sceVideoOutWaitVblank(g_handle);
    }
}
#else
int ml_ui_init(int width, int height) {
    g_w = width;
    g_h = height;
    g_fb = (unsigned int *)calloc((size_t)width * (size_t)height, 4);
    return g_fb ? ML_OK : ML_ERR_MEM;
}
void ml_ui_shutdown(void) {
    free(g_fb);
    g_fb = NULL;
}
void ml_ui_present(void) {}
#endif

void ml_ui_clear(unsigned int rgb) {
    int i, n;
    if (!g_fb) {
        return;
    }
    n = g_w * g_h;
    for (i = 0; i < n; i++) {
        g_fb[i] = 0xFF000000u | rgb;
    }
}

void ml_ui_text(int x, int y, unsigned int rgb, const char *text) {
    int cx = x;
    if (!g_fb || !text) {
        return;
    }
    for (; *text; text++) {
        unsigned char ch = (unsigned char)*text;
        int gx, gy, row;
        if (ch == '\n') {
            y += 10;
            cx = x;
            continue;
        }
        if (ch < 32 || ch > 127) {
            ch = '?';
        }
        for (gy = 0; gy < 8; gy++) {
            row = ML_FONT8[ch - 32][gy];
            for (gx = 0; gx < 8; gx++) {
                if (row & (0x80 >> gx)) {
                    int px = cx + gx * 2;
                    int py = y + gy * 2;
                    int dx, dy;
                    for (dy = 0; dy < 2; dy++) {
                        for (dx = 0; dx < 2; dx++) {
                            int ix = px + dx;
                            int iy = py + dy;
                            if (ix >= 0 && iy >= 0 && ix < g_w && iy < g_h) {
                                g_fb[iy * g_w + ix] = 0xFF000000u | rgb;
                            }
                        }
                    }
                }
            }
        }
        cx += 16;
    }
}
