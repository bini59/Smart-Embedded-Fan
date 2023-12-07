#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <mqueue.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    mqd_t mq;
    const char* mq_name = "/posix_mq";

    // 메시지 큐 오픈
    mq = mq_open(mq_name, O_WRONLY);
    if(mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 소켓 옵션 설정
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 소켓에 주소 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 서버 소켓 대기
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("Waiting for new connection...\n");

        // 클라이언트 연결 수락
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            continue;
        }

        printf("Connection established\n");

        // 클라이언트와 통신
        while(1) {
            memset(buffer, 0, 1024);
            int valread = read(new_socket, buffer, 1024);
            if(valread == 0) {
                printf("Client disconnected\n");
                break;
            }
            printf("Received: %s\n", buffer);
            
            // 메시지 큐로 데이터 전송
            if(mq_send(mq, buffer, strlen(buffer), 0) == -1) {
                perror("mq_send");
                break;
            }
        }

        close(new_socket);
    }

    // 메시지 큐 닫기
    mq_close(mq);

    return 0;
}
