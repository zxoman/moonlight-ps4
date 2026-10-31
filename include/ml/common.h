#ifndef ML_COMMON_H
#define ML_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ML_VERSION "0.1.0"
#define ML_HTTP_PORT 47989
#define ML_HTTPS_PORT 47984
#define ML_RTSP_PORT 48010
#define ML_CTRL_PORT 47995
#define ML_VIDEO_PORT 47998
#define ML_AUDIO_PORT 48000

#define ML_OK 0
#define ML_ERR -1
#define ML_ERR_NET -2
#define ML_ERR_PARSE -3
#define ML_ERR_PAIR -4
#define ML_ERR_PIN -5
#define ML_ERR_MEM -6
#define ML_ERR_UNSUPPORTED -7

#ifndef ML_UNUSED
#define ML_UNUSED(x) (void)(x)
#endif

static inline void *ml_xmalloc(size_t n) {
    void *p = calloc(1, n ? n : 1);
    return p;
}

#endif
