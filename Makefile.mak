CC     = gcc
CFLAGS = -Wall -Wextra
TARGET_NAME = serwer_gg.exe

SRCS = src/main.c \
	   src/hash/md5.c \
       src/http/http_server.c \
       src/http/router.c \
       src/http/http_utils.c \
       src/http/appsvc.c \
       src/http/fmregister.c \
	   src/http/regtoken.c \
	   src/chat/client.c \
	   src/chat/handlers.c \
	   src/chat/protocol.c \
	   src/chat/messaging.c \
	   src/chat/chat_server.c \
	   src/config/config.c \
       src/database/users.c \
	   src/dashboard/dashboard.c

# opcja dla kompilacji dla Windows Server 2003
ifeq ($(RUNNABLE_ON_SERVER_2003), yes)
	CC = i686-w64-mingw32-gcc
    CFLAGS += -D_WIN32_WINNT=0x0502
	LIBS += -static -static-libgcc -Wl,-Bstatic -lws2_32 -lpthread -Wl,-Bdynamic -lmsvcrt -lkernel32
	TARGET_NAME = server_2003_gg.exe
endif

# Detect OS
ifeq ($(OS), Windows_NT)
    TARGET = obj/$(TARGET_NAME)
    LIBS   = -lws2_32
else
    TARGET = obj/serwer_gg
    LIBS   = -lpthread
endif

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)


.PHONY: clean
