#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <mqueue.h>
#include <stdbool.h>

#define BAUD_RATE 115200
static const char *UART2_DEV = "/dev/ttyAMA1";
static const char *MQ_NAME = "/posix_mq";
#define MAX_MSG_SIZE 256
#define AUTO 17

unsigned char serialRead(const int fd);
void serialWrite(const int fd, const unsigned char c);

unsigned char serialRead(const int fd) {
    unsigned char x;
    if (read(fd, &x, 1) != 1)
        return -1;
    return x;
}

void serialWrite(const int fd, const unsigned char c) {
    write(fd, &c, 1);
}

void serialWriteString(const int fd, const char *str) {
    while (*str) {
        write(fd, str, 1);
        str++;
    }
}

bool isExistMode(const int fd_serial, const char *isExistmode) {
    char mode = isExistmode[0];
    char value = isExistmode[1] - '0';
    char output[50];

    switch (mode) {
        case 'Q':
            sprintf(output, "종료\n");
            serialWriteString(fd_serial, output);
            return true;
        case 'T':
            if(value == AUTO)
                sprintf(output, "타이머모드 : 자동\n");
            else
                sprintf(output, "타이머모드 : %d분\n", value * 10);
            serialWriteString(fd_serial, output);
            return true;
        case 'R':
            if(value == AUTO)
                sprintf(output, "회전모드 : 자동\n");
            else
                sprintf(output, "회전모드 : %d단\n", value);
            serialWriteString(fd_serial, output);
            return true;
        case 'P':
            if(value == AUTO)
                sprintf(output, "속도모드 : 자동\n");
            else
                sprintf(output, "속도모드 설정 : %d단\n", value);
            serialWriteString(fd_serial, output);
            return true;
        default:
            return false;
    }
}

int main() {
    int fd_serial;
    unsigned char dat, dat2;
    mqd_t mq;
    struct mq_attr attr;
    char message[MAX_MSG_SIZE];

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    if (wiringPiSetup() < 0)
        return 1;

    if ((fd_serial = serialOpen(UART2_DEV, BAUD_RATE)) < 0) {
        printf("Unable to open serial device.\n");
        return 1;
    }

    while (1) {
        if (serialDataAvail(fd_serial)) {
            dat = serialRead(fd_serial);
            if (serialDataAvail(fd_serial)) {
                dat2 = serialRead(fd_serial);
                sprintf(message, "%c%c", dat, dat2);

                if (isExistMode(fd_serial, message)) {
                    if (mq_send(mq, message, 2, 0) == -1) {
                        perror("mq_send");
                    }
                }
            }
        }
        delay(10);
    }

    mq_close(mq);
    return 0;
}
