#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define BUF_SIZE 1024
#define NAME_SIZE 4

int clnt_cnt = 0;
int clnt_socks[256];
pthread_mutex_t mutx;

char name[NAME_SIZE];
char buf[BUF_SIZE];
struct iovec vec[2];

void* handle_clnt(void* arg);
void send_msg(char* msg, int len);

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    pthread_t t_id;
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    vec[0].iov_base = name;
    vec[0].iov_len = NAME_SIZE + 1;
    vec[1].iov_base = buf;
    vec[1].iov_len = BUF_SIZE;

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr)); // 0으로 초기화
    serv_addr.sin_family = AF_INET; // IPv4 주소체계
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 32비트 IPv4 주소
    serv_addr.sin_port = htons(atoi(argv[1])); // 포트번호

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind() error");
        exit(1);
    }

    if(listen(serv_sock, 5) == -1) {
        perror("listen() error");
        exit(1);
    }

    while(1) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
        pthread_detach(t_id);
        printf("Connected client Port: %d\n", clnt_addr.sin_port);
    }

    close(serv_sock);
    return 0;
}

void* handle_clnt(void* arg) {      // worker thread
    int clnt_sock = *((int*)arg);
    int str_len = 0, i, j;
    int res = 0;
    char msg[BUF_SIZE];
    int operand[BUF_SIZE] = {0, };
    int opCount = 0;
    char oper;


    while(1) {
        readv(clnt_sock, vec, 2);
        opCount = (int)buf[0];
        if(opCount <= 0) {
            break;
        }
        j = sprintf(msg, "[%s]", name);

        for(i = 0; i < opCount; i++) {
            operand[i] = (int)buf[(i * 4) + 1];
        }

        res = operand[0];
        j += sprintf(msg + j, " %d", operand[0]);
        for(i = 0; i < opCount - 1; i++) {
            oper = buf[(opCount * 4) + i + 1];
            switch(oper) {
                case '+':
                    res += operand[i + 1];
                    j += sprintf(msg + j, " + %d", operand[i + 1]);
                    break;
                case '-':
                    res -= operand[i + 1];
                    j += sprintf(msg + j, " - %d", operand[i + 1]);
                    break;
                case '*':
                    res *= operand[i + 1];
                    j += sprintf(msg + j, " * %d", operand[i + 1]);
                    break;
                case '/':
                    res /= operand[i + 1];
                    j += sprintf(msg + j, " / %d", operand[i + 1]);
                    break;
            }
        }

        j += sprintf(msg + j, " = %d\n", res);
        send_msg(msg, j);
    } 

    pthread_mutex_lock(&mutx);
    for(i = 0; i < clnt_cnt; i++) {
        if(clnt_sock == clnt_socks[i]) {
            while(i++ < clnt_cnt - 1) {
                clnt_socks[i] = clnt_socks[i + 1];
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    printf("Closed client\n");
    return NULL;
}

void send_msg(char* msg, int len) {
    int i;
    pthread_mutex_lock(&mutx);
    for(i = 0; i < clnt_cnt; i++) {
        write(clnt_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutx);
}