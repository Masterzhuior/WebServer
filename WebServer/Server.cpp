// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <functional>

#include "Util.h"
#include "base/Logging.h"

/*
    -- Server类的构造函数，用于初始化服务器的属性和状态。
    -- loop 是服务器的事件循环对象。
    -- threadNum 是事件循环线程池的线程数量。
    -- port 是服务器监听的端口号。
    -- eventLoopThreadPool_ 是一个事件循环线程池，用于处理客户端连接的读写事件。
    -- acceptChannel_ 是一个事件通道，用于监听服务器的连接请求。
    -- listenFd_ 是服务器的监听套接字文件描述符。
    -- socket_bind_listen 函数用于创建并绑定服务器的监听套接字，并开始监听连接请求。
    -- acceptChannel_->setFd(listenFd_) 将监听套接字的文件描述符添加到事件通道中。
    -- handle_for_sigpipe() 函数用于处理 SIGPIPE 信号，防止服务器因为客户端异常关闭而崩溃。
    -- setSocketNonBlocking 函数将监听套接字设为非阻塞模式，以提高服务器的性能和可靠性。
*/

/*
    -- start 函数用于启动服务器，并将监听套接字的事件通道添加到事件循环的事件轮询器中。
    -- eventLoopThreadPool_->start() 启动事件循环线程池，开始处理客户端连接的读写事件。
    -- acceptChannel_->setEvents(EPOLLIN | EPOLLET) 设置事件通道的事件类型，包括读事件和边缘触发模式。
    -- acceptChannel_->setReadHandler(bind(&Server::handNewConn, this)) 设置事件通道的读事件处理函数，即当有新的连接请求到达时，执行 handNewConn 函数处理连接请求。
    -- acceptChannel_->setConnHandler(bind(&Server::handThisConn, this)) 设置事件通道的连接事件处理函数，即当连接断开时，执行 handThisConn 函数处理断开事件。
    -- loop_->addToPoller(acceptChannel_, 0) 将事件通道添加到事件循环的事件轮询器中，开始监听连接请求。
    -- started_ = true 表示服务器已经启动完成。
*/


Server::Server(EventLoop *loop, int threadNum, int port)
    : loop_(loop),
      threadNum_(threadNum),
      eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
      started_(false),
      acceptChannel_(new Channel(loop_)),
      port_(port),
      listenFd_(socket_bind_listen(port_)) {
  acceptChannel_->setFd(listenFd_);
  handle_for_sigpipe();
  if (setSocketNonBlocking(listenFd_) < 0) {
    perror("set socket non block failed");
    abort();
  }
}

void Server::start() {
  eventLoopThreadPool_->start();
  // acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
  acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
  acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
  loop_->addToPoller(acceptChannel_, 0);
  started_ = true;
}

void Server::handNewConn() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;

  // 循环调用accept接收客户端的连接请求并返回一个新的套接字文件描述符accept_fd
  while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {
    // 定义一个循环线程池
    EventLoop *loop = eventLoopThreadPool_->getNextLoop();
    LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);
    // cout << "new connection" << endl;
    // cout << inet_ntoa(client_addr.sin_addr) << endl;
    // cout << ntohs(client_addr.sin_port) << endl;
    /*
    // TCP的保活机制默认是关闭的
    int optval = 0;
    socklen_t len_optval = 4;
    getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
    cout << "optval ==" << optval << endl;
    */
    // 限制服务器的最大并发连接数
    if (accept_fd >= MAXFDS) {
      close(accept_fd);
      continue;
    }
    // 设为非阻塞模式
    if (setSocketNonBlocking(accept_fd) < 0) {
      LOG << "Set non block failed!";
      // perror("Set non block failed!");
      return;
    }

    setSocketNodelay(accept_fd);
    // setSocketNoLinger(accept_fd);

    shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
    req_info->getChannel()->setHolder(req_info);
    loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));
  }
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}