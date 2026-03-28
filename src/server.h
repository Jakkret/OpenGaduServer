#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

// ---- Config file reading

typedef struct {
    const char *IPaddr;
    int Port;
} ServerConf;

extern ServerConf sCHAT;
extern ServerConf sHTTP;
#define CONFIG_FILENAME "ogs-conf.ini"

// -------------------------

#define HOST_UNAVAIL	"notoperating"

// -- Buffer sizes ---
#define BUFFER_HTTP  4096
#define BUFFER_CHAT  1024

// --- Terminal colors ---------------
#define RESET    "\033[0m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define RED      "\033[31m"
#define CYAN     "\033[36m"

// -- Logging macros -----------------------
#define LOG_INFO(fmt, ...)  printf(CYAN   "[INFO]  " RESET fmt "\n", ##__VA_ARGS__)
#define LOG_OK(fmt, ...)    printf(GREEN  "[OK]    " RESET fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  printf(YELLOW "[WARN]  " RESET fmt "\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)   printf(RED    "[ERR]   " RESET fmt "\n", ##__VA_ARGS__)

#endif // SERVER_H
