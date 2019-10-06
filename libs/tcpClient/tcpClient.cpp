#include <tcpClient.h>

#include <iostream>
#include <string>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

TcpClient::TcpClient(std::string ip, int port)
{
    init(ip, port);
}

TcpClient::TcpClient(std::string url)
{
    char* pch = strtok((char * const) url.c_str(), ":");
    string ip(pch);
    pch = strtok (NULL, ":");
    int port = atoi(pch);

    init(ip, port);
}

void TcpClient::init(string ip, int port) 
{
    //socket的建立
    this->sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線

    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET;

    info.sin_addr.s_addr = inet_addr(ip.c_str());
    info.sin_port = htons(port);

    int err = connect(this->sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        cerr << "Connection error" << endl;
    }
}

ssize_t TcpClient::sendMsg(char* __buf, size_t __n)
{
    return send(this->sockfd, __buf, __n, 0);
}


ssize_t TcpClient::sendMsg(std::string msg)
{
    char* __buf = (char*) msg.c_str();
    size_t __n = msg.size();
    this->sendMsg(__buf, __n);
}
