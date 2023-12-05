/*
컴파일시
gcc -o 메인_서버 메인_서버.c -lrt -lwiringPi
실행시
./메인_서버
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#define AUTO 17

const char* name = "/posix_mq";
char mode;
int amount;
#include <stdbool.h>

bool isOutOfRange(char mode, int amount) {
    switch (mode) {
        case 'P':
        case 'R':
            if(amount == AUTO)
                return false;
            return (amount < 1 || amount > 3);
        case 'T':
            if(amount == AUTO)
                return false;
            return (amount < 1 || amount > 9);
        default:
            return true; // 알 수 없는 모드
    }
}


int main(int argc, char **argv) {
    mqd_t mq;
    struct mq_attr attr;
    char buf[BUFSIZ];
    int n;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = BUFSIZ;
    attr.mq_curmsgs = 0;
    mq = mq_open(name, O_CREAT | O_RDONLY, 0644, &attr);

    while(1) {
        n = mq_receive(mq, buf, sizeof(buf), NULL);
        if(n > 0) {
            buf[n] = '\0';
            mode = buf[0];
            amount = buf[1] - '0';
            if(isOutOfRange(mode, amount)) {
                continue; // 범위를 벗어난 경우 반복 건너뛰기
            }
            if(mode == 'q') {
                break;
            }
        }
        if (amount == AUTO)
            printf("현재 Mode는 %c Auto 입니다.\n", mode, amount);
        else
            printf("현재 Mode는 %c %d 입니다.\n", mode, amount);
        switch(mode) {
            case 'P':
                break;
            case 'R':
                break;
            case 'T':
                break;
            }
        usleep(100000); // 0.1초 대기
    }

    mq_close(mq);
    mq_unlink(name);
    return 0;
}
