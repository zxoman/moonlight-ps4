#include "ml/input.h"

#include "ml/common.h"

#ifdef ML_PLATFORM_ORBIS
#include <orbis/Pad.h>
#include <orbis/UserService.h>
#endif

/* Shared bit layout used by src/core/app.c */
#define ML_BTN_CROSS 1
#define ML_BTN_CIRCLE 2
#define ML_BTN_OPTIONS 4
#define ML_BTN_TOUCH 8
#define ML_BTN_LEFT 0x10
#define ML_BTN_RIGHT 0x20
#define ML_BTN_UP 0x40
#define ML_BTN_DOWN 0x80

#ifdef ML_PLATFORM_ORBIS
static int g_pad = -1;

int ml_input_init(void) {
    int user = 0;
    sceUserServiceInitialize(NULL);
    scePadInit();
    sceUserServiceGetInitialUser(&user);
    g_pad = scePadOpen(user, 0, 0, NULL);
    return g_pad >= 0 ? ML_OK : ML_ERR;
}

void ml_input_shutdown(void) {
    if (g_pad >= 0) {
        scePadClose(g_pad);
        g_pad = -1;
    }
}

int ml_input_poll(MlPad *out) {
    OrbisPadData data;
    memset(out, 0, sizeof(*out));
    if (g_pad < 0 || !out) {
        return ML_ERR;
    }
    if (scePadReadState(g_pad, &data) < 0) {
        return ML_ERR;
    }
    out->connected = 1;
    out->lx = data.leftStick.x;
    out->ly = data.leftStick.y;
    out->rx = data.rightStick.x;
    out->ry = data.rightStick.y;
    out->l2 = data.analogButtons.l2;
    out->r2 = data.analogButtons.r2;
    if (data.buttons & ORBIS_PAD_BUTTON_CROSS) {
        out->buttons |= ML_BTN_CROSS;
        out->cross = 1;
    }
    if (data.buttons & ORBIS_PAD_BUTTON_CIRCLE) {
        out->buttons |= ML_BTN_CIRCLE;
        out->circle = 1;
    }
    if (data.buttons & ORBIS_PAD_BUTTON_OPTIONS) {
        out->buttons |= ML_BTN_OPTIONS;
        out->options = 1;
    }
    if (data.buttons & ORBIS_PAD_BUTTON_TOUCH_PAD) {
        out->buttons |= ML_BTN_TOUCH;
        out->touchpad = 1;
    }
    if (data.buttons & ORBIS_PAD_BUTTON_LEFT) {
        out->buttons |= ML_BTN_LEFT;
    }
    if (data.buttons & ORBIS_PAD_BUTTON_RIGHT) {
        out->buttons |= ML_BTN_RIGHT;
    }
    if (data.buttons & ORBIS_PAD_BUTTON_UP) {
        out->buttons |= ML_BTN_UP;
    }
    if (data.buttons & ORBIS_PAD_BUTTON_DOWN) {
        out->buttons |= ML_BTN_DOWN;
    }
    return ML_OK;
}
#else
int ml_input_init(void) { return ML_OK; }
void ml_input_shutdown(void) {}
int ml_input_poll(MlPad *out) {
    if (out) {
        memset(out, 0, sizeof(*out));
    }
    return ML_OK;
}
#endif
