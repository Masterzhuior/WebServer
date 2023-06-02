// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "Channel.h"
#include "Epoll.h"
#include "Util.h"
#include "base/CurrentThread.h"
#include "base/Logging.h"
#include "base/Thread.h"


#include <iostream>
using namespace std;

class EventLoop {
 public:
  typedef std::function<void()> Functor;
  // 初始化poller, event_fd，给 event_fd 注册到 epoll 中并注册其事件处理回调
  EventLoop();
  ~EventLoop();

  // 开始事件循环 调⽤该函数的线程必须是该 EventLoop 所在线程，也就是 Loop 函数不能跨线程调⽤
  void loop();
  // 停⽌ Loop
  void quit();

  // 如果当前线程就是创建此EventLoop的线程 就调⽤callback(关闭连接 EpollDel) 否则就放⼊等待执⾏函数区
  void runInLoop(Functor&& cb);
  // 把此函数放⼊等待执⾏函数区 如果当前是跨线程 或者正在调⽤等待的函数则唤醒
  void queueInLoop(Functor&& cb);
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  void assertInLoopThread() { assert(isInLoopThread()); }
  void shutdown(shared_ptr<Channel> channel) { shutDownWR(channel->getFd()); }

  // 从epoll内核事件表中删除fd及其绑定的事件
  void removeFromPoller(shared_ptr<Channel> channel) {
    poller_->epoll_del(channel);
  }

  // 在epoll内核事件表修改fd所绑定的事件
  void updatePoller(shared_ptr<Channel> channel, int timeout = 0) {
    poller_->epoll_mod(channel, timeout);
  }

  // 把fd和绑定的事件注册到epoll内核事件表
  void addToPoller(shared_ptr<Channel> channel, int timeout = 0) {
    poller_->epoll_add(channel, timeout);
  }

 private:
  // 声明顺序 wakeupFd_ > pwakeupChannel_
  
  bool looping_;    // 是否正在事件循环
  shared_ptr<Epoll> poller_;    // io多路复⽤ 分发器
  int wakeupFd_;      // ⽤于异步唤醒 SubLoop 的 Loop 函数中的Poll(epoll_wait因为还没有注册fd会⼀直阻塞)
  bool quit_;         // 是否停⽌事件循环
  bool eventHandling_;     // 是否正在处理事件
  mutable MutexLock mutex_;   // 互斥锁
  std::vector<Functor> pendingFunctors_;  // 正在等待处理的函数
  bool callingPendingFunctors_; // 是否正在调用等待处理的函数
  const pid_t threadId_;    // 线程id
  shared_ptr<Channel> pwakeupChannel_;    // ⽤于异步唤醒的 channel

  void wakeup();      // 异步唤醒SubLoop的epoll_wait(向event_fd中写⼊数据)
  void handleRead();    // eventfd的读回调函数(因为event_fd写了数据，所以触发可读事件，从event_fd读数据)
  void doPendingFunctors();    // 执⾏正在等待的函数(SubLoop注册EpollAdd连接套接字以及绑定事件的函数)
  void handleConn();    // eventfd的更新事件回调函数(更新监听事件)
};
