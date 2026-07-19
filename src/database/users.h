#ifndef USERS_H
#define USERS_H

#include <stdint.h>
#include <time.h>

#define DB_FILE          "users.db"
#define DB_EMAIL_LEN     128
#define DB_PASS_HASH_LEN 64

#define UIN_MIN 10000000
#define UIN_MAX 99999999

typedef struct {
    uint32_t uin;
    char     email[DB_EMAIL_LEN];
    char     password[DB_PASS_HASH_LEN];
    char     qa[256];
    uint32_t registered_at;
    int      active;
} User;

int  db_init();
void db_cleanup();

int  db_register(const char *email, const char *password, const char *qa);

// ZMIANA: out-param zamiast zwracanego wskaźnika (bezpieczne wątkowo)
int  db_find_by_uin(uint32_t uin, User *out);
int  db_find_by_email(const char *email, User *out);

int  db_verify_password(uint32_t uin, const char *password);
void db_dump();

#endif // USERS_H