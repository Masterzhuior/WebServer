#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <unistd.h>
using namespace std;
//全局条件变量
condition_variable cond;
mutex _mutex;
int count = 0;
 
void fun1(){
    while(1)
    {
        count++;
        unique_lock<mutex>lock(_mutex);
        if(count%5 == 0)
        {
            cond.notify_one();
        }
        else
        {
            cout<<"this is fun1,count="<<count<<endl;
        }
        lock.unlock();
        sleep(1);
    }
}
 
void fun2()
{
    while(1)
    {
        unique_lock<mutex>lock(_mutex);
        cond.wait(lock);
        cout<<"this is fun2,count="<<count<<endl;
        lock.unlock();
        sleep(2);
    }
}
 
int main()
{
    thread t1(fun1);
    thread t2(fun2);
    t1.join();
    t2.join();
    return 0;
}