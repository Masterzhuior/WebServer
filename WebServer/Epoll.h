#pragma once
#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"


class Epoll {
 public:
  Epoll();
  ~Epoll();
  void epoll_add(SP_Channel request, int timeout);
  void epoll_mod(SP_Channel request, int timeout);
  void epoll_del(SP_Channel request);
  std::vector<std::shared_ptr<Channel>> poll();
  std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);
  void add_timer(std::shared_ptr<Channel> request_data, int timeout);
  int getEpollFd() { return epollFd_; }
  void handleExpired();

 private:
  static const int MAXFDS = 100000; // 最大文件描述符数量大小
  int epollFd_;
  std::vector<epoll_event> events_;  // epoll_wait()返回的活动事件都放在这个数组里
  std::shared_ptr<Channel> fd2chan_[MAXFDS];  // 负责将文件描述符fd映射到Channel
  std::shared_ptr<HttpData> fd2http_[MAXFDS]; // 负责将文件描述符fd映射到Http地址
  TimerManager timerManager_;  // 时间管理器（管理超时情况）
};