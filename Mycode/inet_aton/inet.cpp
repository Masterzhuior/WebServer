#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>


int main(){
    // inet_ntoa是不可重入的
    struct in_addr addr1, addr2;
    inet_aton("1.2.3.4", &addr1);
    inet_aton("10.194.71.60", &addr2);
    char* szValue1 = inet_ntoa(addr1);
    char* szValue2 = inet_ntoa(addr2);
    printf("address 1:%s\n", szValue1); 
    printf("address 2:%s\n", szValue2);
    /*
    Output:
        address 1:10.194.71.60
        address 2:10.194.71.60 
    */

    return 0;
}