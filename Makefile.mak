CC     = gcc
CFLAGS = -Wall -Wextra

SRCS = src/main.c \
       src/http/http_server.c \
       src/http/router.c \
       src/http/http_utils.c \
       src/http/appsvc.c \
       src/http/fmregister.c \
	   src/chat/client.c \
	   src/chat/handlers.c \
	   src/chat/protocol.c \
	   src/chat/chat_server.c \
       src/database/users.c

# Detect OS
ifeq ($(OS), Windows_NT)
    TARGET = obj/serwer_gg.exe
    LIBS   = -lws2_32
else
    TARGET = serwer_gg
    LIBS   = -lpthread
endif

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean
