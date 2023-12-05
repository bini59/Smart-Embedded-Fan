/*
컴파일시
gcc -o Server Server.c -lrt -lwiringPi
실행시
sudo ./Server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>
#include <stdbool.h>
#include <wiringPi.h>

#define IN_A 18
#define IN_B 19
#define AUTO 17

const char* name = "/posix_mq";
char mode;
int amount;

void setMotor(int speed, int direction) {
    if (direction == 1) {
        digitalWrite(IN_A, HIGH);
        digitalWrite(IN_B, LOW);
    } else {
        digitalWrite(IN_A, LOW);
        digitalWrite(IN_B, HIGH);
    }
    pwmWrite(IN_A, speed);
}

bool isOutOfRange(char mode, int amount) {
    
    switch (mode) {
        case 'Q':
            return false;
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

void init(){
    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi 초기화 실패\n");
        return ;
    }
    pinMode(IN_A, PWM_OUTPUT);
    pinMode(IN_B, OUTPUT);

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

    init();
    
    mq = mq_open(name, O_CREAT | O_RDONLY, 0644, &attr);

    while(1) {
        n = mq_receive(mq, buf, sizeof(buf), NULL);
        buf[n] = '\0';
        mode = buf[0];
        amount = buf[1] - '0';
        if(isOutOfRange(mode, amount)) {
            continue; // 범위를 벗어난 경우 반복 건너뛰기
        }
        // if (amount == AUTO)
        //     printf("현재 Mode는 %c Auto 입니다.\n", mode, amount);
        // else
        //     printf("현재 Mode는 %c %d 입니다.\n", mode, amount);
        switch(mode) {
            case 'Q':
                setMotor(0,1);
                break;
            case 'P':
                if(amount == AUTO){
                    setMotor(0, 1);
                }else
                    setMotor(50*(amount), 1);                    
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
