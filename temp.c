/*
컴파일시
gcc -o Server Server.c -lwiringPi
실행시
sudo ./Server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

void init(){
    if (wiringPiSetupGpio() == -1) {
        printf("WiringPi 초기화 실패\n");
        return ;
    }
    pinMode(IN_A, PWM_OUTPUT);
    pinMode(IN_B, OUTPUT);

}

int main(int argc, char **argv) {

    init();
    setMotor(0, 1);

    return 0;
}
