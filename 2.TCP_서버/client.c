#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char** argv) {
    int sockfd;
    char buf[1024];
    int opCount;
    int opResult;
    struct sockaddr_in servaddr;

    if(argc < 3) {
        printf("Usage: ./client remotePort remoteAddress\n");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket connection failed");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = inet_addr(argv[2]);

    if(connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }

    printf("Operand count: ");
    scanf("%d", &opCount);
    buf[0] = (char)opCount;

    if(buf[0] > 0) {
        for(int i = 0; i < opCount; i++) {
            printf("Operand %d: ", i);
            scanf(" %d", (int*)&buf[(i * 4) + 1]);
        }

        for(int i = 0; i < opCount - 1; i++) {
            printf("Operator %d: ", i);
            scanf(" %c", &buf[(opCount * 4) + i + 1]);
        }

        write(sockfd, buf, 5 * opCount);
        // write(sockfd, buf, 1 + (opCount * 4) + (opCount - 1));

        read(sockfd, &opResult, 4);
        printf("Operation result: %d\n", opResult);
    } else {
        write(sockfd, buf, 1);
    }

    close(sockfd);
    return 0;
}