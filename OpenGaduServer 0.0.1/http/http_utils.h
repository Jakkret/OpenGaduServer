#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

// Extract a parameter value from a query string
// e.g. get_param("fmnumber=12345678&fmt=0", "fmnumber", out, sizeof(out))
// Returns 1 on success, 0 if parameter not found
int get_param(const char *query, const char *param, char *out, int out_size);

// Send a plain HTTP response
void http_send_response(int sock, int status_code, const char *status_text,
                        const char *body);

#endif // HTTP_UTILS_H