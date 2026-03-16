CC     = gcc
CFLAGS = -Wall -Wextra

SRCS = main.c \
       http/http_server.c \
       http/router.c \
       http/http_utils.c \
       http/appsvc.c \
       http/fmregister.c \
       chat/chat_server.c \
	   chat/gg_login.c \
       database/users.c

# Detect OS
ifeq ($(OS), Windows_NT)
    TARGET = serwer_gg.exe
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
