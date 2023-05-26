
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include<sys/stat.h>
#include<cstdlib>
#include<fcntl.h>
//创建守护进程
int ngx_daemon()
{
    int fd;
    switch (fork())//创建子进程
    {
    case -1:
    //创建子进程失败这里可以写日志
        return -1;
    case 0:
       //子进程走到这里直接break即可
        break;
    default:
       exit(0);
       //父进程退出
        break;
    }
    //只有子进程才会走到这里
    if(setsid()==-1)
    {
        //记录错误日志
        return -1;
    }
    umask(0);//设置权限掩码
    fd=open("/dev/null",O_RDWR);//打开黑洞设备以读写方式打开
    if(fd==-1)
    {
         //记录错误日志
         return -1;
    }
    if(dup2(fd,0)==-1)//进行重定向
    {
       //记录错误日志
       return -1;
    }
 
    if(dup2(fd,1)==-1)
    {
        //记录错误日志
        return -1;
    }
    
    if(fd>3)
    {
        if(close(fd)==-1)
        {
            //记录错误日志
            return -1;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    
    if(ngx_daemon()!=1)
    {
        //创建守护进程失败可以做失败后的处理比如写日志等等
        printf("创建守护进程失败！\n");
       return 1;
    }
    else
    {
        //创建守护进程成功执行守护进程要干的活
        for(;;)
        {   
            sleep(1);
            //打印了也没有用因为子进程已经重定向了，不会打印任何结果到标准输出上
            printf("休息一秒,进程id=%d!\n",getpid());
        }
    }
 
    return 0;
}