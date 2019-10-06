#include <udpClient.h>
#include <iostream>

using namespace std;

UdpClient::UdpClient(std::string ip, int port)
{
    init(ip, port);
}

UdpClient::UdpClient(std::string url)
{
    char* pch = strtok((char * const) url.c_str(), ":");
    string ip(pch);
    pch = strtok (NULL, ":");
    int port = atoi(pch);

    init(ip, port);
}

void UdpClient::init(std::string ip, int port)
{
    /* Initialize socket address structure for Interner Protocols */
    bzero(&this->address, sizeof(this->address)); // empty data structure
    this->address.sin_family = AF_INET;
    this->address.sin_addr.s_addr = inet_addr(ip.c_str());
    this->address.sin_port = htons(port);

    /* Create a UDP socket */
    this->socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
}

ssize_t UdpClient::send(const void *__buf, size_t __n)
{
    int out = sendto(this->socket_descriptor, __buf, __n, 0, (struct sockaddr *) &this->address, sizeof(this->address));
    return out;
}