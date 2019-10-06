#include <stdio.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h> 
#include "thread"
#include "vector"
#include "string"

class TcpServer {
public:
    TcpServer(int port);
    void closeServer();
    void closeClient(int id);
    void start();
    void setOnConnection(void (*)(struct sockaddr_in* addr, int));
    void setOnClose(void (*)(struct sockaddr_in* addr, int));
    void setOnMessage(void (*)(struct sockaddr_in* addr, int, std::string));
    bool isAccept();
    void setAccept(bool flag);
    void sendMsg(int id, char* __buf, size_t __n);
    std::vector<int> getClients();
private:
    void init(int port);
    void wait(int sock0);
    void readMessage(struct sockaddr_in* addr, int id);

    int port;
    int sock0;
    int sock_client = -1;
    bool _isAccept;
    std::vector<int> clients;
    std::thread* waitThread;
    std::vector<std::thread*> clientThreads;
    void (*onConnection)(struct sockaddr_in*, int);
    void (*onClose)(struct sockaddr_in* addr, int);
    void (*onMessage)(struct sockaddr_in*, int, std::string);
};