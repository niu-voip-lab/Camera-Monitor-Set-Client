/*
 * Source : http://beej-zhtw.netdpi.net/07-advanced-technology/7-6-broadcast-packet-hello-world
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <ifaddrs.h>
#include <string>
#include <vector>

class UdpBroadcast {
public:
    UdpBroadcast(uint16_t serverPort, char* hostName);
    ~UdpBroadcast();
    void sendMsg(char* __buf, size_t __n);
    static std::vector<std::string> getAddrs();
private:
    int serverPort;
    char* hostName;
    int sockfd;
    struct sockaddr_in their_addr; // 連線者的位址資訊
    void init(uint16_t serverPort, char* hostName);
};