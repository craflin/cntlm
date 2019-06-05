
#include <winsock2.h>

namespace {

class SocketInitializer
{
public:
    SocketInitializer()
    {
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA wsaData;
        WSAStartup(wVersionRequested, &wsaData);
    }
    ~SocketInitializer()
    {
        WSACleanup();
    }
} _socketInitializer;

}
