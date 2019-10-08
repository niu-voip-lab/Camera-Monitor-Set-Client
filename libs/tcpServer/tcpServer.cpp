#include "tcpServer.h"
#include "iostream"

TcpServer::TcpServer(int port)
{
  this->port = port;
}

void TcpServer::start()
{
  init(this->port);
}

void TcpServer::setOnConnection(void (*conn)(struct sockaddr_in *, int))
{
  this->onConnection = conn;
}

void TcpServer::setOnMessage(void (*mesg)(struct sockaddr_in *, int, std::string))
{
  this->onMessage = mesg;
}

void TcpServer::setOnClose(void (*func)(struct sockaddr_in* addr, int id))
{
  onClose = func;
}

void TcpServer::init(int port)
{
  struct sockaddr_in addr;

  /* 製作 socket */
  sock0 = socket(AF_INET, SOCK_STREAM, 0);

  /* 設定 socket */
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sock0, (struct sockaddr *)&addr, sizeof(addr));
  printf("\t[Info] binding...\n");

  /* 設定成等待來自 TCP 用戶端的連線要求狀態 */
  listen(sock0, 5);
  printf("\t[Info] listening...\n");

  /* 接受來自 TCP 用戶端地連線要求*/
  printf("\t[Info] wait for connection...\n");

  waitThread = new std::thread(&TcpServer::wait, this, sock0);
}

void TcpServer::readMessage(struct sockaddr_in *addr, int id)
{
  while (true)
  {
    int n;
    char buf[4096];
    std::string msg("");

    n = read(id, buf, sizeof(buf));
    if(n == 0)
    {
      onClose(addr, id);
      break;
    }
    msg += std::string(buf);
    onMessage(addr, id, msg);
  }
}

void TcpServer::wait(int sock0)
{
  socklen_t len;
  struct sockaddr_in client;
  len = sizeof(client);

  int temp_sock_client;
  while(true)
  {
    temp_sock_client = accept(sock0, (struct sockaddr *)&client, &len);
    if (!_isAccept)
    {
      close(temp_sock_client);
      return;
    }
    sock_client = temp_sock_client;
    clients.push_back(sock_client);
    onConnection(&client, temp_sock_client);

    std::thread *thd = new std::thread(&TcpServer::readMessage, this, &client, temp_sock_client);
    clientThreads.push_back(thd);
  }
}

void TcpServer::sendMsg(int id, char* __buf, size_t __n)
{
  write(id, __buf, __n);
}

void TcpServer::sendMsg(int id, std::string msg)
{
  char *__buf = (char*) msg.c_str();
  write(id, __buf, msg.size());
}

void TcpServer::closeClient(int id)
{
  close(id);
}

void TcpServer::closeServer()
{
  /* 結束 listen 的 socket */
  printf("\t[Info] Close self connection...\n");
  close(sock0);
}

void TcpServer::setAccept(bool flag)
{
  _isAccept = flag;
}

bool TcpServer::isAccept()
{
  return _isAccept;
}