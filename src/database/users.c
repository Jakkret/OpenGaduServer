/* 
    Przepisanie implementacji bazy danych z własnego na sqlite3
    Jakkret - 19 Lipca 2026 @ 20:03
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sqlite/sqlite3.h"
#include "../server.h"
#include "users.h"

static sqlite3 *db;

// -- Password hashing -- bez zmian, ten sam djb2
static void hash_password(const char *password, char *out_hash) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*password++)) {
        hash = ((hash << 5) + hash) + c;
    }
    snprintf(out_hash, DB_PASS_HASH_LEN, "%lu", hash);
}

static int uin_is_taken(uint32_t uin) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT 1 FROM users WHERE uin=? AND active=1;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, (int)uin);
    int taken = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return taken;
}

static uint32_t generate_uin() {
    uint32_t uin;
    int attempts = 0;
    do {
        uin = (rand() % (UIN_MAX - UIN_MIN + 1)) + UIN_MIN;
        if (++attempts > 1000) {
            LOG_ERR("DB: Failed to generate unique UIN after 1000 attempts");
            return 0;
        }
    } while (uin_is_taken(uin));
    return uin;
}

// ── Init & cleanup ────────────────────────────────────────
int db_init() {
    srand((unsigned int)time(NULL));

    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        LOG_ERR("DB: sqlite3_open failed: %s", sqlite3_errmsg(db));
        return -1;
    }

    const char *ddl =
        "CREATE TABLE IF NOT EXISTS users ("
        "uin INTEGER PRIMARY KEY,"
        "email TEXT UNIQUE,"
        "password_hash TEXT NOT NULL,"
        "qa TEXT,"
        "registered_at INTEGER NOT NULL,"
        "active INTEGER NOT NULL DEFAULT 1);";

    char *errmsg = NULL;
    if (sqlite3_exec(db, ddl, NULL, NULL, &errmsg) != SQLITE_OK) {
        LOG_ERR("DB: schema create failed: %s", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }

    // WAL - lepsza wydajność przy wielu wątkach
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);

    LOG_OK("DB: sqlite3 opened (%s)", DB_FILE);
    return 0;
}

void db_cleanup() {
    if (db) sqlite3_close(db);
    LOG_INFO("DB: Database closed");
}

// -- Core operations ------------------
int db_register(const char *email, const char *password, const char *qa) {
    if (!password) {
        LOG_ERR("DB: db_register() called with NULL arguments");
        return -1;
    }

    if (email && email[0]) {
        User tmp;
        if (db_find_by_email(email, &tmp)) {
            LOG_WARN("DB: email already exists: %s", email);
            return -1;
        }
    }

    uint32_t uin = generate_uin();
    if (uin == 0) return -1;

    char hash[DB_PASS_HASH_LEN];
    hash_password(password, hash);

    sqlite3_stmt *stmt;
    const char *sql =
        "INSERT INTO users (uin,email,password_hash,qa,registered_at,active) "
        "VALUES (?,?,?,?,?,1);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, (int)uin);
    if (email && email[0]) sqlite3_bind_text(stmt, 2, email, -1, SQLITE_TRANSIENT);
    else sqlite3_bind_null(stmt, 2);
    sqlite3_bind_text(stmt, 3, hash, -1, SQLITE_TRANSIENT);
    if (qa) sqlite3_bind_text(stmt, 4, qa, -1, SQLITE_TRANSIENT);
    else sqlite3_bind_null(stmt, 4);
    sqlite3_bind_int(stmt, 5, (int)time(NULL));

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        LOG_ERR("DB: Failed to save after registering UIN %u: %s", uin, sqlite3_errmsg(db));
        return -1;
    }

    LOG_OK("DB: Registered new user — UIN: %u, email: %s", uin, email ? email : "");
    return (int)uin;
}

static int row_to_user(sqlite3_stmt *stmt, User *out) {
    memset(out, 0, sizeof(User));
    out->uin = (uint32_t)sqlite3_column_int(stmt, 0);

    const unsigned char *email = sqlite3_column_text(stmt, 1);
    if (email) strncpy(out->email, (const char*)email, DB_EMAIL_LEN - 1);

    const unsigned char *pass = sqlite3_column_text(stmt, 2);
    if (pass) strncpy(out->password, (const char*)pass, DB_PASS_HASH_LEN - 1);

    const unsigned char *qa = sqlite3_column_text(stmt, 3);
    if (qa) strncpy(out->qa, (const char*)qa, sizeof(out->qa) - 1);

    out->registered_at = (uint32_t)sqlite3_column_int(stmt, 4);
    out->active        = sqlite3_column_int(stmt, 5);
    return 1;
}

int db_find_by_uin(uint32_t uin, User *out) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "SELECT uin,email,password_hash,qa,registered_at,active "
        "FROM users WHERE uin=? AND active=1;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, (int)uin);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        row_to_user(stmt, out);
        found = 1;
    }
    sqlite3_finalize(stmt);
    return found;
}

int db_find_by_email(const char *email, User *out) {
    if (!email) return 0;

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "SELECT uin,email,password_hash,qa,registered_at,active "
        "FROM users WHERE email=? AND active=1;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        row_to_user(stmt, out);
        found = 1;
    }
    sqlite3_finalize(stmt);
    return found;
}

int db_verify_password(uint32_t uin, const char *password) {
    User u;
    if (!db_find_by_uin(uin, &u)) {
        LOG_WARN("DB: Password check failed — UIN %u not found", uin);
        return 0;
    }

    char hash[DB_PASS_HASH_LEN];
    hash_password(password, hash);

    int match = (strncmp(u.password, hash, DB_PASS_HASH_LEN) == 0);

    if (match) LOG_OK("DB: Password verified for UIN %u", uin);
    else       LOG_WARN("DB: Wrong password for UIN %u", uin);

    return match;
}

// -- Debug -------------------
void db_dump() {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT uin,email,active FROM users;", -1, &stmt, NULL);

    printf("\n── Database dump ──────────────\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        uint32_t uin = (uint32_t)sqlite3_column_int(stmt, 0);
        const unsigned char *email = sqlite3_column_text(stmt, 1);
        int active = sqlite3_column_int(stmt, 2);
        printf("  UIN: %-10u  email: %-30s  active: %d\n",
            uin, email ? (const char*)email : "-", active);
    }
    sqlite3_finalize(stmt);
    printf("────────────────────────────────\n\n");
}