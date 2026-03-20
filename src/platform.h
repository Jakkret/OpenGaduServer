#ifndef PLATFORM_H
#define PLATFORM_H
/*
 * 		Platform.h - Platform detection
 * Detects on which platform is the source code 
 * compiled and adds these fixes below to make 
 * code work on Linux and Win32
 *
 */


// --- Detect platform ----------------
#ifdef _WIN32
    #define PLATFORM_WINDOWS

    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    // On Windows, link with -lws2_32
    #pragma comment(lib, "ws2_32.lib")

    // Map POSIX socket names to Winsock equivalents
    #define close(s)        closesocket(s)
    #define socklen_t       int

    // Winsock doesn't have these, define them manually
    #ifndef INET_ADDRSTRLEN
        #define INET_ADDRSTRLEN 16
    #endif

#else
    #define PLATFORM_LINUX

    #include <sys/socket.h>
    #include <sys/select.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

// ----- Threads -----------------------------
#ifdef PLATFORM_WINDOWS
    typedef HANDLE thread_t;

    #define thread_create(t, func, arg) \
        (*(t) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(func), (arg), 0, NULL), *(t) != NULL ? 0 : -1)

    #define thread_join(t) \
        WaitForSingleObject(t, INFINITE)
		
	#define thread_detach(t) \
		CloseHandle(t)

#else
    #include <pthread.h>
    typedef pthread_t thread_t;

    #define thread_create(t, func, arg) \
        pthread_create(t, NULL, func, arg)

    #define thread_join(t) \
        pthread_join(t, NULL)
	#define thread_detach(t) \
		pthread_detach(t)
#endif

// --- Socket initialization ------------------------
#ifdef PLATFORM_WINDOWS
    static inline int platform_init() {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            return -1;
        }
        return 0;
    }

    static inline void platform_cleanup() {
        WSACleanup();
    }
#else
    static inline int platform_init()    { return 0; }
    static inline void platform_cleanup() {}
#endif

// -- byte padding ----------------
#ifdef PLATFORM_WINDOWS
    #define GG_PACKED_BEGIN	_Pragma("pack(push, 1)")
    #define GG_PACKED_END	_Pragma("pack(pop)")
    #define GG_PACKED		__attribute__((packed))		// for MINGW compiler, the one I use
#else
    #define GG_PACKED_BEGIN
    #define GG_PACKED_END
    #define GG_PACKED        __attribute__((packed))
#endif

#endif // PLATFORM_H