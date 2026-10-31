#ifndef ML_LOG_H
#define ML_LOG_H

void ml_log_init(const char *udp_host, int udp_port);
void ml_log(const char *fmt, ...);

#endif
