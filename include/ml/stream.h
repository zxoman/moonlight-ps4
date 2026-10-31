#ifndef ML_STREAM_H
#define ML_STREAM_H

typedef struct MlStreamStats {
    unsigned long packets;
    unsigned long bytes;
    unsigned long nalus;
    unsigned long fu_drops;
    int last_nal_type;
} MlStreamStats;

int ml_stream_open(int port);
void ml_stream_close(void);
int ml_stream_poll(int timeout_ms, MlStreamStats *stats);

#endif
