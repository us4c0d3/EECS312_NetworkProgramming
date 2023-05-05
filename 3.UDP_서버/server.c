#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_SIZE 1024

int main(int argc, char** argv) {
    int sockfd, cSockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len, str_len;

    char buf[MAX_SIZE];

    if(argc < 2) {
        printf("usage: ./server localPort\n");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    int enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = 0;
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        return -1;
    }


    while(1) {
        len = sizeof(cliaddr);
        str_len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr, &len);
        int opCount = 0;
        opCount = (int)buf[0];
        if(opCount <= 0) {
            printf("server close(%d)\n", opCount);
            break;
        }
        
        int res = 0;
        int operand[MAX_SIZE];
        char operator;

        printf("Operand count: %d\n", opCount);
        for(int i = 0; i < opCount; i++) {
            operand[i] = (int)buf[(i * 4) + 1];
            printf("Operand %d: %d\n", i, operand[i]);
        }
        res += operand[0];
        for(int i = 0; i < opCount - 1; i++) {
            operator = buf[(opCount * 4) + i + 1];
            printf("Operator %d: %c\n", i, operator);
            switch(operator) {
                case '+':
                    res += operand[i + 1];
                    break;
                case '-':
                    res -= operand[i + 1];
                    break;
                case '*':
                    res *= operand[i + 1];
                    break;
            }
        }

        printf("Operation result: %d\n", res);
        sendto(sockfd, &res, sizeof(int), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    }

    close(cSockfd);
    close(sockfd);
    return 0;
}