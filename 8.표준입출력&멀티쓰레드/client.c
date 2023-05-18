#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define NAME_SIZE 4

void* send_msg(void* arg);
void* recv_msg(void* arg);

char name[NAME_SIZE];
char buf[BUF_SIZE];

struct iovec vec[2];


int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
    if(argc != NAME_SIZE) {
        printf("Usage: %s <port> <IP> <name>\n", argv[0]);
        exit(1);
    }
    if(strlen(argv[3]) != NAME_SIZE) {
        printf("ID have to be 4\n");
        exit(1);
    }

    strcpy(name, argv[3]);
    name[4] = '\0';

    vec[0].iov_base = name;
    vec[0].iov_len = NAME_SIZE + 1;
    vec[1].iov_base = buf;
    vec[1].iov_len = BUF_SIZE;

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr)); // 0으로 초기화
    serv_addr.sin_family = AF_INET; // IPv4 주소체계
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]); // 32비트 IPv4 주소
    serv_addr.sin_port = htons(atoi(argv[1])); // 포트번호

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect() error");
        return -1;
    }

    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;

}

void* send_msg(void* arg) {
    int sock = *((int*)arg);
    int opCount;
    while(1) {
        memset(buf, 0, BUF_SIZE);
        scanf("%d", &opCount);
        buf[0] = (char)opCount;
        if(buf[0] <= 0) {
            printf("Overflow Number(%d) - Closed client\n", buf[0]);
            writev(sock, vec, 2);
            close(sock);
            exit(0);
        }
        
        // input operand
        for(int i = 0; i < opCount; i++) {
            scanf(" %d", (int*)&buf[(i * 4) + 1]);
        }

        // input operator
        for(int i = 0; i < opCount - 1; i++) {
            scanf(" %c", &buf[(opCount * 4) + i + 1]);
        }

        // send
        writev(sock, vec, 2);
    }
    return NULL;
}

void* recv_msg(void* arg) {
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;
    while(1) {
        str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
        if(str_len == -1) {
            return (void*)-1;
        }
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}