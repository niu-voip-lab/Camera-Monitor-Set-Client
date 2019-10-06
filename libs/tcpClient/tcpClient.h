/**
 * Source:
 * http://zake7749.github.io/2015/03/17/SocketProgramming/
*/
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef PRINTER_H_
#define PRINTER_H_
class TcpClient
{
public:
    TcpClient(std::string, int);
    TcpClient(std::string);
    ssize_t sendMsg(char* __buf, size_t __n);
    ssize_t sendMsg(std::string msg);
private:
    int sockfd = 0;
    void init(std::string, int);
};
#endif