#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <mqueue.h>

#define PORT 8000

int sock;
int value;
struct sockaddr_in serv_addr;

void main() {
    mqd_t mq;
    struct mq_attr attr;
    char buf[BUFSIZ];
    int n;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = BUFSIZ;
    attr.mq_curmsgs = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "172.20.10.2", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }
    


    const char* mq_name = "/posix_RA";
    mq = mq_open(mq_name, O_CREAT | O_RDONLY, 0644, &attr);

    while(1) {
        n = mq_receive(mq, buf, sizeof(buf), NULL);
        printf("%c receive ! \n",buf[0]);
        buf[n] = '\0';
        char message[50];
        sprintf(message, "%c",buf[0]);
        if (send(sock, message, strlen(message), 0) < 0) {
            printf("Failed to send message\n");
        } else {
            printf("Sent: %s\n", message);
        }
    }



}