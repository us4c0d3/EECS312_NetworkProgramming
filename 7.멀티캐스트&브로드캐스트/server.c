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

int main(int argc, char *argv[]) {
    if(!strcmp(argv[1], "discovery")) {     // discovery server
        //TODO
        int discovery_sock;
        int str_len;
        char buf[BUF_SIZE];
        int so_brd=1;
        struct sockaddr_in broad_adr;

        discovery_sock=socket(PF_INET, SOCK_DGRAM, 0);
        memset(&broad_adr, 0, sizeof(broad_adr));
        broad_adr.sin_family=AF_INET;
        broad_adr.sin_addr.s_addr=inet_addr("255.255.255.255");
        broad_adr.sin_port=htons(DISCOVERY_PORT);

        setsockopt(discovery_sock, SOL_SOCKET, SO_BROADCAST, (void*)&so_brd, sizeof(so_brd));

        printf("Discovery Server operating...\n");

        recvfrom(discovery_sock, buf, BUF_SIZE, 0, NULL, 0);
        


    }
    else if(!strcmp(argv[1], "calc")) {     // calc server
        //TODO
        printf("Regist calc server\n");
        srand(time(NULL));
        int port = (int)(rand() % 40001 + 10000);
        
        int calc_sock;
        struct sockaddr_in calc_adr;
        int str_len;
        char buf[BUF_SIZE];
        int result;

        calc_sock=socket(PF_INET, SOCK_DGRAM, 0);
        memset(&calc_adr, 0, sizeof(calc_adr));
        calc_adr.sin_family=AF_INET;
        

    }
    else {
        printf("Usage : %s <discovery|calc>\n", argv[0]);
        exit(1);
    }
}