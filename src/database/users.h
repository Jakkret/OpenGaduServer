#ifndef USERS_H
#define USERS_H

#include <stdint.h>
#include <time.h>

// Constants
#define DB_FILE          "users.dat"
#define DB_MAX_USERS     10000
#define DB_EMAIL_LEN     128
#define DB_PASS_HASH_LEN 64

#define UIN_MIN 10000000
#define UIN_MAX 99999999

// User Record
typedef struct {
    uint32_t uin;                           // 8-digit unique user number
    char     email[DB_EMAIL_LEN];      	    // optional (v5) -- user's email address
    char     password[DB_PASS_HASH_LEN]; 	// hashed password
	char 	 qa[256];						// security question (v5)
    uint32_t registered_at;        		    // unix timestamp of registration
    int      active;                		// 1 = active, 0 = deleted
} User;

// DB state
typedef struct {
    User     users[DB_MAX_USERS];
    int      count;
} UserDatabase;


// Initialize database — load from file if exists
int  db_init();

// Save database to file
int  db_save();

// Free memory and close database
void db_cleanup();

// Register a new user — returns assigned UIN, or -1 on failure
int  db_register(const char *email, const char *password, const char *qa);

// Find user by UIN — returns pointer or NULL
User* db_find_by_uin(uint32_t uin);

// Find user by email — returns pointer or NULL
User* db_find_by_email(const char *email);

// Verify password for a given UIN — returns 1 if correct, 0 if not
int  db_verify_password(uint32_t uin, const char *password);

// Print all users to stdout (debug)
void db_dump();

#endif // USERS_H