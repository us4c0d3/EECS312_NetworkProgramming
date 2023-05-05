#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_SIZE 1024

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: ./client remotePort remoteAddress OR ./client domainName\n");
        return -1;
    }

    if(argc == 2) {
        struct hostent *host, *host2;
        struct sockaddr_in addr;

        host = gethostbyname(argv[1]);
        if(!host) {
            perror("gethostbyname() error");
            return -1;
        }

        printf("gethostbyname()\n");
        printf("Official name: %s\n", host->h_name);
        for(int i = 0; host->h_aliases[i]; i++) {
            printf("Aliases %d: %s\n", i, host->h_aliases[i]);
        }
        printf("Address type: %s\n", (host->h_addrtype==AF_INET) ? "AF_INET" : "AF_INET6");
        for(int i = 0; host->h_addr_list[i]; i++) {
            printf("IP addr %d: %s\n", i, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
        }

        memset(&addr, 0, sizeof(addr));
        addr.sin_addr = *(struct in_addr*)host->h_addr_list[0];
        host2 = gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);
        if(!host2) {
            perror("gethostbyaddr() error");
            return -1;
        }

        printf("\ngethostbyaddr()\n");
        printf("Official name: %s\n", host2->h_name);
        for(int i = 0; host2->h_aliases[i]; i++) {
            printf("Aliases %d: %s\n", i, host2->h_aliases[i]);
        }
        printf("Address type: %s\n", (host2->h_addrtype==AF_INET) ? "AF_INET" : "AF_INET6");
        for(int i = 0; host2->h_addr_list[i]; i++) {
            printf("IP addr %d: %s\n", i, inet_ntoa(*(struct in_addr*)host2->h_addr_list[i]));
        }


    } else {
        int sockfd;
        char buf[MAX_SIZE];
        struct sockaddr_in servaddr;
        int sockType, len, state;
        FILE* fp;
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket connection failed");
            return -1;
        }

        len = sizeof(sockType);
        state = getsockopt(sockfd, SOL_SOCKET, SO_TYPE, (void *)&sockType, &len);
        if(state) {
            perror("getsockopt() error");
            return -1;
        }
        printf("This socket type is %s\n", (sockType == 1) ? "SOCK_STREAM" : "SOCK_DGRAM");
        printf("This sock type is %d\n", sockType);
        
        if(!(fp = fopen("copy.txt", "w+"))) {
            perror("file open error");
            return -1;
        }

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atoi(argv[1]));
        servaddr.sin_addr.s_addr = inet_addr(argv[2]);

        if(connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            perror("connect error");
            return -1;
        }

        read(sockfd, buf, sizeof(buf));
        printf("%s", buf);
        fputs(buf, fp);

        close(sockfd);
    }

    return 0;
}