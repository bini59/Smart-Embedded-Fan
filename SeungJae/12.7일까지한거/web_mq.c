#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mqueue.h>

int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Usage: %s <command>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFSIZ];
    sprintf(buffer, "%s", argv[1]);
    mqd_t mq;
    const char* mq_name = "/posix_mq";

    // 메시지 큐 오픈
    mq = mq_open(mq_name, O_WRONLY);
    if(mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
        
    // 메시지 큐로 데이터 전송
    if(mq_send(mq, buffer, strlen(buffer), 0) == -1) {
        perror("mq_send");
    }

    // 메시지 큐 닫기
    mq_close(mq);

    return 0;
}