#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <arpa/inet.h>

#define MAX_SIZE 1024

void read_childproc(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
    if(pid == -1) {
        perror("waitpid failed");
    }
}


int main(int argc, char** argv) {
    int sockfd, cSockfd;
    int fd_save[2], fd_load[2];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    pid_t pid;
    char buf[MAX_SIZE];
    struct sigaction act;
    int str_len, status, fd_max, fd_num;
    fd_set reads, cpy_reads;
    struct iovec vec[3];

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if(argc < 2) {
        printf("usage: %s localPort\n", argv[0]);
        return -1;
    }

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    int enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    servaddr.sin_family = PF_INET;
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

    if(pipe(fd_save) < 0 || pipe(fd_load) < 0) {
        perror("pipe failed");
        return -1;
    }
    pid = fork();
    if(pid == 0) {      // child process
        char mode[5], id[5];
        char log[MAX_SIZE] = {0, };
        char* arr[MAX_SIZE][2];
        int log_sz = 0;
        int flag;
        while(1) {
            read(fd_save[0], mode, 5);
            if(!strcmp(mode, "quit")) {
                break;
            } else if(!strcmp(mode, "save")) {
                flag = -1;
                read(fd_save[0], id, 5);
                for(int i = 0; i < log_sz; i++) {
                    if(!strcmp(arr[i][0], id)) {
                        flag = i;
                        break;
                    }
                }
                memset(log, 0, MAX_SIZE);
                read(fd_save[0], log, MAX_SIZE);
                if(flag == -1) {
                    arr[log_sz][0] = malloc(sizeof(char) * 5);
                    arr[log_sz][1] = malloc(sizeof(char) * MAX_SIZE);
                    memset(arr[log_sz][1], 0, MAX_SIZE);
                    strcpy(arr[log_sz][0], id);
                    strcpy(arr[log_sz][1], log);
                    arr[log_sz][1][strlen(arr[log_sz][1])] = '\0';
                    log_sz++;
                } else {
                    arr[flag][1][strlen(arr[flag][1])] = '\n';
                    arr[flag][1][strlen(arr[flag][1]) + 1] = '\0';
                    strcat(arr[flag][1], log);
                }
            } else if(!strcmp(mode, "load")) {
                flag = -1;
                read(fd_save[0], id, 5);
                for(int i = 0; i < log_sz; i++) {
                    if(!strcmp(arr[i][0], id)) {
                        flag = i;
                        break;
                    }
                }
                if(flag == -1) {
                    write(fd_load[1], "Not exist", 10);
                } else {
                    write(fd_load[1], arr[flag][1], MAX_SIZE);
                }
            }
        }
        return 0;
    }

    FD_ZERO(&reads);
    FD_SET(sockfd, &reads);
    fd_max = sockfd;

    char mode[5], id[5];
    char log[MAX_SIZE];
    int opCount;
    int operand[MAX_SIZE] = {0, };
    int q = 0;
    vec[0].iov_base = mode;
    vec[0].iov_len = 5;
    vec[1].iov_base = id;
    vec[1].iov_len = 5;
    vec[2].iov_base = buf;
    vec[2].iov_len = MAX_SIZE;

    while(1) {
        cpy_reads=reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) {
            perror("select");
            break;
        }

        if(fd_num == 0)
            continue;

        for(int i = 0; i < fd_max + 1; i++) {
            if(FD_ISSET(i, &cpy_reads)) {
                if(i == sockfd) {
                    len = sizeof(cliaddr);
                    cSockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
                    FD_SET(cSockfd, &reads);
                    if(fd_max < cSockfd)
                        fd_max = cSockfd;
                    printf("connected client: %d \n", cSockfd);
                } else {
                    str_len = readv(i, vec, 3);
                    if(str_len == 0) {
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client %d\n", i);
                        if(q == 1) {
                            close(cSockfd);
                            close(sockfd);
                            return 0;
                        }
                    } else {
                        write(fd_save[1], mode, 5);
                        if(!strcmp(mode, "save")) {  // save
                            printf("save to %s\n", id);
                            write(fd_save[1], id, 5);
                            opCount = (int)buf[0];
                            int j = sprintf(log, "%s: ", id);
                            int res = 0;
                            char operator;

                            for(int i = 0; i < opCount; i++) {
                                operand[i] = (int)buf[(i * 4) + 1];
                            }
                            res += operand[0];
                            j += sprintf(log + j, "%d", operand[0]);
                            for(int i = 0; i < opCount - 1; i++) {
                                operator = buf[(opCount * 4) + i + 1];
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
                            j += sprintf(log + j, " = %d", res);
                            log[j] = '\0';
                            write(fd_save[1], log, j);
                            write(cSockfd, &res, 4);
                        } else if(!strcmp(mode, "load")) {   // load
                            memset(log, 0, MAX_SIZE);
                            printf("load from %s\n", id);
                            write(fd_save[1], id, 4);
                            read(fd_load[0], log, MAX_SIZE);
                            write(cSockfd, log, strlen(log));
                        } else if(!strcmp(mode, "quit")) {    // quit
                            printf("quit\n");
                            close(fd_save[1]);
                            close(fd_load[1]);
                            // wait(&status);
                            sleep(5);
                            q = 1;
                        }
                    }
                }
            } 
        }
    }
    close(cSockfd);
    close(sockfd);
    return 0;
}