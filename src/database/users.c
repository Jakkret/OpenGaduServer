#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../server.h"
#include "users.h"

// ---- Internal database instance -----------------
static UserDatabase db;

// -- Password hashing --------------------------
// Simple djb2-based hash — good enough for local/dev use
// TODO: replace with proper bcrypt or SHA-256 for production

static void hash_password(const char *password, char *out_hash) {
    unsigned long hash = 5381;
    int c;

    while ((c = (unsigned char)*password++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }

    snprintf(out_hash, DB_PASS_HASH_LEN, "%lu", hash);
}

// ----- UIN generation -------------------------
static int uin_is_taken(uint32_t uin) {
    for (int i = 0; i < db.count; i++) {
        if (db.users[i].active && db.users[i].uin == uin) {
            return 1;
        }
    }
    return 0;
}

static uint32_t generate_uin() {
    uint32_t uin;
    int attempts = 0;

    do {
        uin = (rand() % (UIN_MAX - UIN_MIN + 1)) + UIN_MIN;
        attempts++;

        if (attempts > 1000) {
            LOG_ERR("DB: Failed to generate unique UIN after 1000 attempts");
            return 0;
        }
    } while (uin_is_taken(uin));

    return uin;
}

// ── Init & cleanup ────────────────────────────────────────
int db_init() {
    memset(&db, 0, sizeof(db));
    srand((unsigned int)time(NULL));

    FILE *f = fopen(DB_FILE, "rb");
    if (!f) {
        LOG_WARN("DB: No database file found — starting fresh (%s)", DB_FILE);
        return 0;
    }

    // Read user count
    if (fread(&db.count, sizeof(int), 1, f) != 1) {
        LOG_ERR("DB: Failed to read user count from file");
        fclose(f);
        return -1;
    }

    // Sanity check
    if (db.count < 0 || db.count > DB_MAX_USERS) {
        LOG_ERR("DB: Corrupted database — invalid user count: %d", db.count);
        fclose(f);
        db.count = 0;
        return -1;
    }

    // Read all user records
    if (fread(db.users, sizeof(User), db.count, f) != (size_t)db.count) {
        LOG_ERR("DB: Failed to read user records from file");
        fclose(f);
        db.count = 0;
        return -1;
    }

    fclose(f);
    LOG_OK("DB: Loaded %d user(s) from %s", db.count, DB_FILE);
    return 0;
}

int db_save() {
    FILE *f = fopen(DB_FILE, "wb");
    if (!f) {
        LOG_ERR("DB: Failed to open %s for writing", DB_FILE);
        return -1;
    }

    // Write user count
    if (fwrite(&db.count, sizeof(int), 1, f) != 1) {
        LOG_ERR("DB: Failed to write user count");
        fclose(f);
        return -1;
    }

    // Write all user records
    if (fwrite(db.users, sizeof(User), db.count, f) != (size_t)db.count) {
        LOG_ERR("DB: Failed to write user records");
        fclose(f);
        return -1;
    }

    fclose(f);
    LOG_OK("DB: Saved %d user(s) to %s", db.count, DB_FILE);
    return 0;
}

void db_cleanup() {
    db_save();
    memset(&db, 0, sizeof(db));
    LOG_INFO("DB: Database closed");
}

// -- Core operations ------------------
int db_register(const char *email, const char *password, const char *qa) {
    if (!password) {
        LOG_ERR("DB: db_register() called with NULL arguments");
        return -1;
    }
	
    // Check capacity
    if (db.count >= DB_MAX_USERS) {
        LOG_ERR("DB: Database full (%d users)", DB_MAX_USERS);
        return -1;
    }

    // Generate unique UIN
    uint32_t uin = generate_uin();
    if (uin == 0) return -1;

    // Fill in the new user record
    User *u = &db.users[db.count];
    memset(u, 0, sizeof(User));

    u->uin           = uin;
    u->registered_at = (uint32_t)time(NULL);
    u->active        = 1;

    strncpy(u->password, password, DB_PASS_HASH_LEN - 1);

    // Change for the Email to be checked optionally - if exists then..
	if(email && email[0]){
		if (db_find_by_email(email) != NULL) {
			LOG_WARN("DB: email already exists: %s", email);
			return -1;
		}
		strncpy(u->email, email, DB_EMAIL_LEN - 1);
	}


    db.count++;

    // Persist to disk immediately
    if (db_save() != 0) {
        LOG_ERR("DB: Failed to save after registering UIN %u", uin);
        db.count--;  // rollback
        return -1;
    }

    LOG_OK("DB: Registered new user — UIN: %u, email: %s", uin, email);
    return (int)uin;
}

User* db_find_by_uin(uint32_t uin) {
    for (int i = 0; i < db.count; i++) {
        if (db.users[i].active && db.users[i].uin == uin) {
            return &db.users[i];
        }
    }
    return NULL;
}

User* db_find_by_email(const char *email) {
    if (!email) return NULL;

    for (int i = 0; i < db.count; i++) {
        if (db.users[i].active &&
            strncmp(db.users[i].email, email, DB_EMAIL_LEN) == 0) {
            return &db.users[i];
        }
    }
    return NULL;
}

int db_verify_password(uint32_t uin, const char *password) {
    User *u = db_find_by_uin(uin);
    if (!u) {
        LOG_WARN("DB: Password check failed — UIN %u not found", uin);
        return 0;
    }

    char hash[DB_PASS_HASH_LEN];
    hash_password(password, hash);

    int match = (strncmp(u->password, hash, DB_PASS_HASH_LEN) == 0);

    if (match) {
        LOG_OK("DB: Password verified for UIN %u", uin);
    } else {
        LOG_WARN("DB: Wrong password for UIN %u", uin);
    }

    return match;
}

// -- Debug -------------------
void db_dump() {
    printf("\n── Database dump (%d users) ──────────────\n", db.count);
    for (int i = 0; i < db.count; i++) {
        User *u = &db.users[i];
        printf("  [%d] UIN: %-10u  email: %-30s  active: %d\n",
            i, u->uin, u->email, u->active);
    }
    printf("──────────────────────────────────────────\n\n");
}
