#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_SIZE 128

int main(int argc, char** argv) {
    int sockfd, cSockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    len = sizeof(cliaddr);

    if(argc < 2) {
        printf("usage: ./server localPort\n");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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

    if(listen(sockfd, 5) < 0) {
        perror("socket failed");
        return -1;
    }

    while(1) {
        if((cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept error");
            return -1;
        }

        char buf;
        int opCount = 0;
        read(cSockfd, &buf, 1);
        opCount = (int)buf;
        if(opCount <= 0) {
            printf("server close(%d)\n", opCount);
            break;
        }
        
        int res = 0;
        int operand[MAX_SIZE];
        char operator;

        printf("Operand count: %d\n", opCount);
        for(int i = 0; i < opCount; i++) {
            read(cSockfd, &operand[i], 4);
            printf("Operand %d: %d\n", i, operand[i]);
        }
        res += operand[0];
        for(int i = 0; i < opCount - 1; i++) {
            read(cSockfd, &operator, 1);
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
        write(cSockfd, &res, 4);
    }

    close(cSockfd);
    close(sockfd);
    return 0;
}