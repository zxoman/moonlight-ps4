#ifndef ML_INPUT_H
#define ML_INPUT_H

typedef struct MlPad {
    int connected;
    unsigned int buttons;
    int lx, ly, rx, ry;
    int l2, r2;
    int cross, circle, options, touchpad;
} MlPad;

int ml_input_init(void);
void ml_input_shutdown(void);
int ml_input_poll(MlPad *pad);

#endif
