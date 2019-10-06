/*
* sender.c--UDP protocol example
* https://sites.google.com/site/yunluliussite/linux-shell/linux-c-udp-socket-example
*/
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

class UdpClient
{
public:
    UdpClient(std::string, int);
    UdpClient(std::string);
    ssize_t send(const void *__buf, size_t __n);
private:
    int socket_descriptor;
    struct sockaddr_in address;
    void init(std::string, int);
};