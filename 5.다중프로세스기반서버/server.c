#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define MAX_SIZE 1024

void read_childproc(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
}

int main(int argc, char** argv) {
    int sockfd, cSockfd;
    int fds[2];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    pid_t pid;
    struct sigaction act;
    int str_len, state;

    if(argc < 2) {
        printf("usage: ./server localPort\n");
        return -1;
    }

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

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

    if(pipe(fds) < 0) {
        perror("pipe failed");
        return -1;
    }
    pid = fork();
    if(pid == 0) {
        FILE* fp;
        char buf[MAX_SIZE];
        int len;
        if((fp = fopen("log.txt", "w")) == NULL) {
            perror("fopen error");
            return -1;
        }
        while(1) {
            len = read(fds[0], buf, MAX_SIZE);
            if(buf[0] <= 0) {
                break;
            }
            fwrite((void*)buf, 1, len, fp);
        }
        fclose(fp);
        return 0;
    }

    while(1) {
        len = sizeof(cliaddr);
        if((cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            continue;
        } else
            puts("new client connected...");


        pid = fork();
        if(pid == 0) {
            char buf;
            int opCount = 0;
            read(cSockfd, &buf, 1);
            opCount = (int)buf;
            if(opCount <= 0) {
                printf("Save File(%d)\n", opCount);
                write(fds[1], &buf, 1);
            } else {
                char log[MAX_SIZE];
                int j = sprintf(log, "%d: ", getpid());
                int res = 0;
                int operand[MAX_SIZE];
                char operator;

                printf("Operand count: %d\n", opCount);
                for(int i = 0; i < opCount; i++) {
                    read(cSockfd, &operand[i], 4);
                    printf("Operand %d: %d\n", i, operand[i]);
                }
                res += operand[0];
                j += sprintf(log + j, "%d", operand[0]);
                for(int i = 0; i < opCount - 1; i++) {
                    read(cSockfd, &operator, 1);
                    printf("Operator %d: %c\n", i, operator);
                    switch(operator) {
                        case '+':
                            j += sprintf(log + j, " + %d", operand[i + 1]);
                            res += operand[i + 1];
                            break;
                        case '-':
                            j += sprintf(log + j, " - %d", operand[i + 1]);
                            res -= operand[i + 1];
                            break;
                        case '*':
                            j += sprintf(log + j, " * %d", operand[i + 1]);
                            res *= operand[i + 1];
                            break;
                    }
                }
                j += sprintf(log + j, " = %d\n", res);
                printf("%s", log);
                write(fds[1], log, j);
                write(cSockfd, &res, 4);
            }
            
            close(cSockfd);
            return 0;
        } else {
            close(cSockfd);
        }
        
    }

    close(cSockfd);
    close(sockfd);
    return 0;
}