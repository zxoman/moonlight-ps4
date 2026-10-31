#include "ml/ui.h"

#include "ml/common.h"
#include "font8.h"

static unsigned int *g_fb;
static int g_w, g_h;
static unsigned long g_frames;

int ml_ui_width(void) { return g_w; }
int ml_ui_height(void) { return g_h; }

int ml_ui_init(int width, int height) {
    g_w = width;
    g_h = height;
    g_fb = (unsigned int *)calloc((size_t)width * (size_t)height, 4);
    g_frames = 0;
    return g_fb ? ML_OK : ML_ERR_MEM;
}

void ml_ui_shutdown(void) {
    free(g_fb);
    g_fb = NULL;
}

void ml_ui_present(void) {
    g_frames++;
    if ((g_frames % 60) == 1) {
        fprintf(stdout, "[ui] frame %lu\n", g_frames);
        fflush(stdout);
    }
    if (g_frames > 3) {
        /* Host --ui is a smoke path, not an interactive emulator. */
        exit(0);
    }
}

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
                    int ix = cx + gx;
                    int iy = y + gy;
                    if (ix >= 0 && iy >= 0 && ix < g_w && iy < g_h) {
                        g_fb[iy * g_w + ix] = 0xFF000000u | rgb;
                    }
                }
            }
        }
        cx += 8;
    }
}
