#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main() {

    // 1.创建一个通信的socket
    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if(fd == -1) {
        perror("socket");
        exit(-1);
    }   

    // 2.设置多播属性
    struct in_addr multi_addr;
    inet_pton( AF_INET , "239.0.0.10" , &multi_addr.s_addr);
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &multi_addr, sizeof(multi_addr));
    
    // 3.创建一个多播的地址
    struct sockaddr_in cliaddr;
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(9999);
    inet_pton(AF_INET, "239.0.0.10", &cliaddr.sin_addr.s_addr);

    // 3.通信
    int num = 0;
    while(1) {
       
        char sendBuf[128];
        sprintf(sendBuf, "hello, client....%d", num++);
        // 发送数据
        int len = sendto(fd, sendBuf, strlen(sendBuf) + 1, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
        if( len == strlen(sendBuf) + 1){
            printf("多播的数据 : %s\n", sendBuf);
        }
        sleep(1);
    }

    close(fd);
    return 0;
}