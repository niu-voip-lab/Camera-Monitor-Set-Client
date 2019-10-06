#include "udpBroadcast.h"
#include "iostream"
#include "memory"

using namespace std;

UdpBroadcast::UdpBroadcast(uint16_t serverPort, char* hostName)
{
    init(serverPort, hostName);
}

UdpBroadcast::~UdpBroadcast()
{
    close(sockfd);
}

std::vector<std::string> UdpBroadcast::getAddrs()
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);
    vector<string> addrs;

    int ctn = 0;

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            
            string buf(addressBuffer);
            std::size_t found = buf.find_last_of(".");
            buf = buf.substr(0,found+1) + "255";
            addrs.push_back(buf);
        }
    }
    return addrs;
}

void UdpBroadcast::init(uint16_t serverPort, char* hostName)
{
    struct hostent *he;
    int broadcast = 1;

    if ((he=gethostbyname(hostName)) == NULL) { // 取得 host 資訊
        perror("gethostbyname");
        return;
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return;
    }

    // 這個 call 就是要讓 sockfd 可以送廣播封包
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        return;
    }

    their_addr.sin_family = AF_INET; // host byte order
    their_addr.sin_port = htons(serverPort); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
}

void UdpBroadcast::sendMsg(char* __buf, size_t __n)
{
    int numbytes;
    if ((numbytes=sendto(sockfd, __buf, __n, 0, (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
        perror("sendto");
        return;
    }
}