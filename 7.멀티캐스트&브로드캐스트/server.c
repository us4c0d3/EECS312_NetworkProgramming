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
#include <time.h>

#define DISCOVERY_PORT 8080
#define CALC_PORT 8081
#define CLIENT_PORT 8082

#define BUF_SIZE 1024

void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage : %s <discovery|calc>\n", argv[0]);
        exit(1);
    }
    int so_brd=1;
    if(!strcmp(argv[1], "discovery")) {     // discovery server
        int discovery_sock, calc_sock, client_sock;
        int str_len;
        char buf[BUF_SIZE];
        struct sockaddr_in broad_adr, calc_adr, client_adr, recv_adr;
        int recv_adr_sz, recv_port;
        char *calc_port = NULL;

        // discovery socket prepare
        discovery_sock=socket(PF_INET, SOCK_DGRAM, 0);
        memset(&broad_adr, 0, sizeof(broad_adr));
        broad_adr.sin_family=AF_INET;
        broad_adr.sin_addr.s_addr=inet_addr("255.255.255.255");
        broad_adr.sin_port=htons(DISCOVERY_PORT);

        setsockopt(discovery_sock, SOL_SOCKET, SO_BROADCAST, (void*)&so_brd, sizeof(so_brd));

        printf("Discovery Server operating...\n");
        if(bind(discovery_sock, (struct sockaddr*)&broad_adr, sizeof(broad_adr))==-1)
            error_handling("bind() error");


        // calc socket prepare
        calc_sock=socket(PF_INET, SOCK_DGRAM, 0);
        memset(&calc_adr, 0, sizeof(calc_adr));
        calc_adr.sin_family=AF_INET;
        calc_adr.sin_addr.s_addr=inet_addr("255.255.255.255");
        calc_adr.sin_port=htons(CALC_PORT);


        // client socket prepare
        client_sock=socket(PF_INET, SOCK_DGRAM, 0);
        memset(&client_adr, 0, sizeof(client_adr));
        client_adr.sin_family=AF_INET;
        client_adr.sin_addr.s_addr=inet_addr("255.255.255.255");
        client_adr.sin_port=htons(CLIENT_PORT);

        memset(&recv_adr, 0, sizeof(recv_adr));
        recv_adr_sz = sizeof(recv_adr);

        while(1) {
            str_len = recvfrom(discovery_sock, buf, BUF_SIZE, 0, (struct sockaddr *)&recv_adr, &recv_adr_sz);
            if(str_len < 0) {
                break;
            }

            recv_port = ntohs(recv_adr.sin_port);
            if(recv_port == CALC_PORT) {
                if(calc_port == NULL) {
                    calc_port = (char*)malloc(sizeof(char) * 6);
                    for(int i = 0; i < 5; i++) {
                        calc_port[i] = buf[i + 7];
                    }
                    calc_port[5] = '\0';
                    printf("Calc Server registered(%s)\n", calc_port);
                    sendto(discovery_sock, "success", strlen("success"), 0, (struct sockaddr*)&calc_adr, sizeof(calc_adr));
                } else {
                    sendto(discovery_sock, "fail", strlen("fail"), 0, (struct sockaddr*)&calc_adr, sizeof(calc_adr));
                }
            } else if(recv_port == CLIENT_PORT) {
                if(calc_port == NULL) {
                    sendto(discovery_sock, "fail", strlen("fail"), 0, (struct sockaddr*)&client_adr, sizeof(client_adr));
                } else {
                    sendto(discovery_sock, calc_port, strlen(calc_port), 0, (struct sockaddr*)&client_adr, sizeof(client_adr));
                }
            }
        }

        close(discovery_sock);
        close(calc_sock);
        close(client_sock);
        return 0;
    }
    else if(!strcmp(argv[1], "calc")) {     // calc server
        srand(time(NULL));
        int port = (int)(rand() % 40001 + 10000);
        printf("Register calc server\n");
        
        int calc_sock, discovery_sock, client_sock;
        struct sockaddr_in calc_adr, send_adr, discovery_adr, client_adr;
        char buf[BUF_SIZE];
        char serv_port[13] = "";
        struct sigaction act;
        int str_len, status, fd_max, fd_num;
        fd_set reads, cpy_reads;
        struct timeval timeout;
        int opCount = 0;
        int operand[BUF_SIZE] = {0, };

        sprintf(serv_port, "server:%d", port);

        // discovery socket prepare
        discovery_sock = socket(PF_INET, SOCK_DGRAM, 0);
        memset(&discovery_adr, 0, sizeof(discovery_adr));
        discovery_adr.sin_family=AF_INET;
        discovery_adr.sin_addr.s_addr=inet_addr("255.255.255.255");
        discovery_adr.sin_port=htons(DISCOVERY_PORT);

        memset(&send_adr, 0, sizeof(send_adr));
        send_adr.sin_family=AF_INET;
        send_adr.sin_addr.s_addr=inet_addr("255.255.255.255");
        send_adr.sin_port=htons(CALC_PORT);

        setsockopt(discovery_sock, SOL_SOCKET, SO_BROADCAST, (void*)&so_brd, sizeof(so_brd));

        if(bind(discovery_sock, (struct sockaddr*)&send_adr, sizeof(send_adr))==-1)
            error_handling("Fail");


        if(sendto(discovery_sock, serv_port, strlen(serv_port), 0, (struct sockaddr*)&discovery_adr, sizeof(discovery_adr)) < 0) {
            perror("sendto() error");
            exit(1);
        }
        socklen_t discovery_adr_sz = sizeof(discovery_adr);
        recvfrom(discovery_sock, buf, BUF_SIZE, 0, (struct sockaddr*)&discovery_adr, &discovery_adr_sz);
        if(!strcmp(buf, "success")) {
            printf("Calc Server(%d) operating...\n", port);
        } else {
            printf("Fail\n");
            exit(1);
        }

        // calc socket prepare (tcp socket)
        calc_sock = socket(PF_INET, SOCK_STREAM, 0);
        memset(&calc_adr, 0, sizeof(calc_adr));
        calc_adr.sin_family=AF_INET;
        calc_adr.sin_addr.s_addr=htonl(INADDR_ANY);
        calc_adr.sin_port=htons(port);

        if(bind(calc_sock, (struct sockaddr*)&calc_adr, sizeof(calc_adr))==-1)
            error_handling("bind() error");

        if(listen(calc_sock, 5)==-1)
            error_handling("listen() error");

        FD_ZERO(&reads);
        FD_SET(calc_sock, &reads);
        fd_max = calc_sock;

        // multiflexing server
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
                    if(i == calc_sock) {
                        int adr_sz = sizeof(client_adr);
                        client_sock = accept(calc_sock, (struct sockaddr*)&client_adr, &adr_sz);
                        FD_SET(client_sock, &reads);
                        if(fd_max < client_sock)
                            fd_max = client_sock;
                        printf("connected client: %d\n", client_sock);
                    } else {
                        char tmp;
                        str_len = read(i, &tmp, 1);
                        if(str_len == 0) {      // close request
                            FD_CLR(i, &reads);
                            close(i);
                            printf("closed client: %d\n", i);
                        } else {    // main logic
                            opCount = (int)tmp;
                            for(int j = 0; j < opCount; j++) {
                                read(i, &operand[j], 4);
                            }

                            int res = operand[0];
                            for(int j = 1; j < opCount; j++) {
                                read(i, &tmp, 1);
                                switch(tmp) {
                                    case '+':
                                        res += operand[j];
                                        break;
                                    case '-':
                                        res -= operand[j];
                                        break;
                                    case '*':
                                        res *= operand[j];
                                        break;
                                    case '/':
                                        res /= operand[j];
                                        break;
                                }
                            }
                            printf("Operation result: %d\n", res);
                            write(i, &res, 4);
                        }
                    }
                }
            }      
        }
    }
    else {
        printf("Usage : %s <discovery|calc>\n", argv[0]);
        exit(1);
    }
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_childproc(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
    if(pid == -1) {
        perror("waitpid failed");
    }
}