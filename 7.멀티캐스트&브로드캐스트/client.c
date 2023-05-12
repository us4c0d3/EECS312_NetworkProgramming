#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DISCOVERY_PORT 8080
#define CALC_PORT 8081
#define PORT 8082

#define BUF_SIZE 1024

int main(int argc, char** argv) {
    int discovery_sockfd, sockfd;
    char buf[BUF_SIZE] = {0, };
    int opCount;
    int opResult;
    struct sockaddr_in calc_adr, discovery_adr, recv_adr;
    int so_brd=1;
    socklen_t recv_adr_sz;

    // discovery socket prepare
    discovery_sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&discovery_adr, 0, sizeof(discovery_adr));
    discovery_adr.sin_family = AF_INET;
    discovery_adr.sin_addr.s_addr = inet_addr("255.255.255.255");
    discovery_adr.sin_port = htons(DISCOVERY_PORT);

    memset(&recv_adr, 0, sizeof(recv_adr));
    recv_adr.sin_family = AF_INET;
    recv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    recv_adr.sin_port = htons(PORT);
    recv_adr_sz = sizeof(recv_adr);

    setsockopt(discovery_sockfd, SOL_SOCKET, SO_BROADCAST, (void*)&so_brd, sizeof(so_brd));

    if(bind(discovery_sockfd, (struct sockaddr*)&recv_adr, recv_adr_sz) == -1) {
        perror("bind() error");
        return -1;
    }

    printf("Start to find calc server\n");
    if(sendto(discovery_sockfd, "client", strlen("client"), 0, (struct sockaddr*)&discovery_adr, sizeof(discovery_adr)) == -1) {
        perror("sendto() error");
        return -1;
    }
    recvfrom(discovery_sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&recv_adr, &recv_adr_sz);
    printf("Received calc server port: %s\n", buf);
    if(!strcmp(buf, "fail")) {
        printf("FAIL\n");
        return -1;
    }
    int port = atoi(buf);
    printf("Found calc server(%d)\n", port);
    close(discovery_sockfd);

    memset(buf, 0, BUF_SIZE);
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket connection failed");
        return -1;
    }

    // calc socket prepare
    calc_adr.sin_family = AF_INET;
    calc_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
    calc_adr.sin_port = htons(port);

    printf("Operand count: ");
    scanf("%d", &opCount);
    buf[0] = (char)opCount;

    if(buf[0] > 0) {
        if(connect(sockfd, (const struct sockaddr*)&calc_adr, sizeof(calc_adr)) < 0) {
            perror("connect error");
            return -1;
        }

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
    }

    close(sockfd);
    return 0;
}