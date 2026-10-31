#ifndef ML_HTTP_H
#define ML_HTTP_H

int ml_http_get(const char *host, int port, int tls, const char *path_and_query, char **body,
                int *body_len);

#endif
