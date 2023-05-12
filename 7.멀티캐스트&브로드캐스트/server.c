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

int main(int argc, char *argv[]) {
    if(!strcmp(argv[1], "discovery")) {     // discovery server
        //TODO
        int discovery_sock, calc_sock, client_sock;
        int str_len;
        char buf[BUF_SIZE];
        int so_brd=1;
        struct sockaddr_in broad_adr, calc_adr, client_adr, recv_adr;
        int recv_adr_sz, recv_port;
        char calc_port[6] = NULL;

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
        calc_adr.sin_addr.s_addr=INADDR_ANY;
        calc_adr.sin_port=htons(CALC_PORT);


        // client socket prepare
        client_sock=socket(PF_INET, SOCK_DGRAM, 0);
        memset(&client_adr, 0, sizeof(client_adr));
        client_adr.sin_family=AF_INET;
        client_adr.sin_addr.s_addr=INADDR_ANY;
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
                    for(int i = 0; i < 5; i++) {
                        calc_port[i] = buf[i + 6];
                    }
                    printf("Calc Server registerd(%s)\n", calc_port);
                    sendto(discovery_sock, "success", strlen("success"), 0, (struct sockaddr*)&calc_adr, sizeof(calc_adr));
                } else {
                    sendto(discovery_sock, "fail", strlen("fail"), 0, (struct sockaddr*)&calc_adr, sizeof(calc_adr));
                }
            } else if(recv_port == CLIENT_PORT) {
                if(calc_port == NULL) {
                    sendto(discovery_sock, "fail", strlen("fail"), 0, (struct sockaddr*)&client_adr, sizeof(client_adr));
                } else {
                    sendto(discovery_sock, calc_port, strlen(calc_port), 0, (struct sockaddr*)&clent_adr, sizeof(client_adr));
                }
            } else {
                printf("Unknown port\n");
            }
        }

        close(discovery_sock);
        close(calc_sock);
        close(client_sock);
        return 0;
    }
    else if(!strcmp(argv[1], "calc")) {     // calc server
        //TODO
        printf("Regist calc server\n");
        srand(time(NULL));
        int port = (int)(rand() % 40001 + 10000);
        
        int calc_sock, regist_sock;
        struct sockaddr_in calc_adr;
        int str_len;
        char buf[BUF_SIZE];
        int result;

        // from receiver.c
        // recv_sock=socket(PF_INET, SOCK_DGRAM, 0);
        // memset(&adr, 0, sizeof(adr));
        // adr.sin_family=AF_INET;
        // adr.sin_addr.s_addr=htonl(INADDR_ANY);
        // adr.sin_port=htons(atoi(argv[1]));

        regist_sock = socket(PF_INET, SOCK_DGRAM, 0);
        memset(&regist_sock, 0, sizeof(regist_sock));
        regist_sock.sin_family=AF_INET;
        regist_sock.sin_addr.s_addr=htonl(INADDR_ANY);
        regist_sock.sin_port=htons(CALC_PORT);

        if(bind(regist_sock, (struct sockaddr*)&regist_sock, sizeof(regist_sock))==-1)
            error_handling("bind() error");

        // multiflexing server
        while(1) {
            //TODO
            str_len=recvfrom(recv_sock, buf, BUF_SIZE-1, 0, NULL, 0);
            if(str_len<0)
                break;
            buf[str_len]=0;
            fputs(buf, stdout);
            
            
            
            
            
            
            
            
            
            
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