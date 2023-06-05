#pragma once
#include <sys/epoll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"

class EventLoop;
class HttpData;

class Channel {
 private:
  typedef std::function<void()> CallBack;
  EventLoop *loop_;
  int fd_;
  __uint32_t events_;
  __uint32_t revents_;
  __uint32_t lastEvents_;

  // 方便找到上层持有该Channel的对象
  // weak_ptr是⼀个观测者（不会增加或减少引⽤计数）,同时也没有重载->,和*等运算符 所以不能直接使⽤
  // 可以通过lock函数得到它的shared_ptr（对象没销毁就返回，销毁了就返回空shared_ptr）
  // expired函数判断当前对象是否销毁了
  std::weak_ptr<HttpData> holder_;  

 private:
  int parse_URI();
  int parse_Headers();
  int analysisRequest();

  CallBack readHandler_;
  CallBack writeHandler_;
  CallBack errorHandler_;
  CallBack connHandler_;

 public:
  Channel(EventLoop *loop);
  Channel(EventLoop *loop, int fd);
  ~Channel();
  int getFd();
  void setFd(int fd);
  // 返回weak_ptr所指向的shared_ptr对象
  void setHolder(std::shared_ptr<HttpData> holder) { holder_ = holder; }
  std::shared_ptr<HttpData> getHolder() {
    std::shared_ptr<HttpData> ret(holder_.lock());
    return ret;
  }

  // 设置回调函数
  void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
  void setWriteHandler(CallBack &&writeHandler) {
    writeHandler_ = writeHandler;
  }
  void setErrorHandler(CallBack &&errorHandler) {
    errorHandler_ = errorHandler;
  }
  void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }


  // IO事件回调函数的调⽤接⼝
  // EventLoop中调⽤Loop开始事件循环 会调⽤Poll得到就绪事件
  // 然后依次调⽤此函数处理就绪事件
  void handleEvents() {
    events_ = 0;
    // 触发挂起事件 并且没触发可读事件
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
      events_ = 0;
      return;
    }
    // 触发错误事件
    if (revents_ & EPOLLERR) {
      if (errorHandler_) errorHandler_();
      events_ = 0;
      return;
    }
    // 触发可读事件 | ⾼优先级可读 | 对端（客户端）关闭连接
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      handleRead();
    }
    // 触发可写事件
    if (revents_ & EPOLLOUT) {
      handleWrite();
    }
    //处理更新监听事件(EpollMod)
    handleConn();
  }

  void handleRead();    // 处理读事件的回调
  void handleWrite();   // 处理写事件的回调
  void handleError(int fd, int err_num, std::string short_msg);   // 处理错误事件的回调
  void handleConn();

  void setRevents(__uint32_t ev) { revents_ = ev; }

  void setEvents(__uint32_t ev) { events_ = ev; }
  __uint32_t &getEvents() { return events_; }

  bool EqualAndUpdateLastEvents() {
    bool ret = (lastEvents_ == events_);
    lastEvents_ = events_;
    return ret;
  }

  __uint32_t getLastEvents() { return lastEvents_; }
};

typedef std::shared_ptr<Channel> SP_Channel;