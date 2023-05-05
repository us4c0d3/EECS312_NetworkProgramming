#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>

#define MAX_SIZE 1024

int main(int argc, char** argv) {
    int sockfd;
    struct iovec vec[3];
    char* mode = malloc(MAX_SIZE);
    char* id = malloc(MAX_SIZE);
    char buf[MAX_SIZE] = {0, };
    int opCount;
    int opResult;
    struct sockaddr_in servaddr;
    int flag;   // 0: save, 1: load, 2: quit

    memset(mode, 0, MAX_SIZE);
    memset(id, 0, MAX_SIZE);

    if(argc < 3) {
        printf("Usage: %s remotePort remoteAddress\n", argv[0]);
        return -1;
    }

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket connection failed");
        return -1;
    }

    servaddr.sin_family = PF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = inet_addr(argv[2]);

    printf("Mode: ");
    scanf(" %s", mode);
    vec[0].iov_base=mode;
    vec[0].iov_len=5;
    vec[1].iov_base=id;
    vec[1].iov_len=5;
    vec[2].iov_base=buf;
    vec[2].iov_len=1;
    if(!strcmp(mode, "save") || !strcmp(mode, "load")) {
        mode = realloc(mode, 5);
        printf("ID: ");
        scanf(" %s", id);
        if(strlen(id) != 4) {
            printf("Error: ID length must be 4\n");
            return -1;
        }
        id = realloc(id, 5);
        flag = !strcmp(mode, "save") ? 0 : 1;
    } else if(!strcmp(mode, "quit")) {
        mode = realloc(mode, 5);
        flag = 2;
    } else {
        printf("supported mode: save load quit\n");
        return -1;
    }

    if(connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }

    if(flag == 0) {     // save
        printf("Operand count: ");
        scanf(" %d", &opCount);
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
            vec[2].iov_len = 5 * opCount;
            writev(sockfd, vec, 3);
            read(sockfd, &opResult, 4);
            printf("Operation result: %d\n", opResult);
        } else {
            printf("Overflow will happen(%d)", buf[0]);
            close(sockfd);
            return -1;
        }
    } else {        // load, quit
        writev(sockfd, vec, 3);
        if(flag == 1) {    // load
            read(sockfd, buf, MAX_SIZE);
            printf("%s\n", buf);
        }
    }

    close(sockfd);
    return 0;
}