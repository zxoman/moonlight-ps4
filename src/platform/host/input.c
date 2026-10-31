#include "ml/input.h"

#include "ml/common.h"

int ml_input_init(void) { return ML_OK; }

void ml_input_shutdown(void) {}

int ml_input_poll(MlPad *out) {
    if (!out) {
        return ML_ERR;
    }
    memset(out, 0, sizeof(*out));
    return ML_OK;
}
