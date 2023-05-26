#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
static bool stop = false;

static void handle_term(int sig) { stop = true; }

int main(int argc, char* argv[]) {
    /*
    1. 创建socket
    int socket(int domain, int type, int protocol);
        -- domain参数告诉系统使用哪个底层协议族。对TCP/IP协议族而言，
            该参数应该设置为PF_INET（Protocol Family of Internet，用于IPv4）或PF_INET6（用于IPv6）； 对于UNIX本地域协议族而言，该
            参数应该设置为PF_UNIX。关于socket系统调用支持的所有协议族，
            请读者自己参考其man手册。

        --
    type参数指定服务类型。服务类型主要有SOCK_STREAM服务（流服务）和SOCK_UGRAM（数据报）服务。
            对TCP/IP协议族而言，其值取SOCK_STREAM表示传输层使用TCP协议，取
            SOCK_DGRAM表示传输层使用UDP协议。

        -- protocol参数是在前两个参数构成的协议集合下，再选择一个具体的协议。
            不过这个值通常都是唯一的（前两个参数已经完全决定了它的值）。几乎在所有情况下，我们都应该把它设置为0，表示使用默认协议。

            socket系统调用成功时返回一个socket文件描述符，失败则返回-1并设置errno。

    2. 命名socket
    int bind(int sockfd, const struct sockaddr*my_addr, socklen_t addrlen);
        bind将my_addr所指的socket地址分配给未命名的sockfd文件描述符，addrlen参数指出该socket地址的长度。

    3. 监听socket
    int listen(int sockfd,int backlog);
        -- backlog参数提示内核监听队列的最大长度。
    */

    signal(SIGTERM, handle_term);
    if (argc <= 3) {
        printf("usage:%s ip_address port_number backlog\n", basename(argv[0]));
        return 1;
    }

    // IP地址，端口号，backlog初始化
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    // 创建socket，IPv4，TCP协议
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    /*创建一个IPv4 socket地址*/
    struct sockaddr_in address;

    // 将地址结构体 address 的内存清零，以避免出现未初始化的内存数据。
    bzero(&address, sizeof(address));  
    // 设置地址结构体的地址族为 IPv4。
    address.sin_family = AF_INET;    

    // 将 IP 地址的字符串表示形式转换为网络字节序的二进制形式，并存储在地址结构体 address 中。 
    inet_pton(AF_INET, ip, &address.sin_addr);      

    // 将端口号转换为网络字节序，并存储在地址结构体 address 中。
    address.sin_port = htons(port);     

    // 将地址结构体 address 绑定到套接字 sock 上，以便在指定的 IP 地址和端口号上监听连接请求。函数的返回值 ret 表示绑定操作是否成功。
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));     
    assert(ret != -1);

    // 开始监听套接字 sock 上的连接请求，backlog 参数指定了连接请求队列的最大长度。函数的返回值 ret 表示监听操作是否成功。
    ret = listen(sock, backlog);
    assert(ret != -1);

    /*循环等待连接，直到有SIGTERM信号将它中断*/
    while (!stop) {
    sleep(1);
    }

    /*关闭socket，见后文*/
    close(sock);
    return 0;
}