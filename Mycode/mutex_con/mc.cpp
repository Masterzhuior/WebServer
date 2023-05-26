#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string.h>
#include <vector>
#include <thread>

constexpr int REQUIRED_REQUESTS = 3;
std::mutex mtx;
std::condition_variable cv;
int request_count = 0;

void process_requests(int id) {
    std::unique_lock<std::mutex> lock(mtx);
    ++request_count;
    std::cout << "线程 " << id << " 发出请求，当前请求数：" << request_count << std::endl;

    // 当未达到3个请求时，等待其他线程发出请求
    while (request_count < REQUIRED_REQUESTS) {
        cv.wait(lock);
    }

    // 当达到3个请求时，处理所有请求
    std::cout << "线程 " << id << " 处理请求\n";

    // 处理完成后，唤醒其他等待的线程
    cv.notify_all();
}

int main() {
    const int THREAD_COUNT = 20;
    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(process_requests, i);
    }

    for (auto &t : threads) {
        t.join();
    }

    return 0;
}