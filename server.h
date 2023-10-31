#include <sys/socket.h>
#include <netinet/in.h>

struct server
{
    sockaddr_in addr;
    int addr_len;
    int port;

};
