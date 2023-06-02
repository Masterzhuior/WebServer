#include <getopt.h>
#include <string>
#include "EventLoop.h"
#include "Server.h"
#include "base/Logging.h"


int main(int argc, char *argv[]) {
    // 定义了线程数和端口号以及log的存放位置
    int threadNum = 4;
    int port = 80;
    std::string logPath = "./WebServer.log";

    // parse args
    int opt;
    const char *str = "t:l:p:";
    while ((opt = getopt(argc, argv, str)) != -1) {
      switch (opt) {
        case 't': {
          threadNum = atoi(optarg);
          break;
        }
        case 'l': {
          logPath = optarg;
          if (logPath.size() < 2 || optarg[0] != '/') {
            printf("logPath should start with \"/\"\n");
            abort();
          }
          break;
        }
        case 'p': {
          port = atoi(optarg);
          break;
        }
        default:
          break;
      }
    }
    
    Logger::setLogFileName(logPath);
    // STL库在多线程上应用
    #ifndef _PTHREADS
    LOG << "_PTHREADS is not defined !";
    #endif

    // 实例化一个主循环对象，初始化WebServer的单例对象，最后启动WebServer并开启主循环
    EventLoop mainLoop;
    Server myHTTPServer(&mainLoop, threadNum, port);
    myHTTPServer.start();
    mainLoop.loop();
    return 0;
}
