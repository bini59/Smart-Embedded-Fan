#include <stdio.h>
#include <wiringPi.h>
#include <signal.h>
#include <stdlib.h>

#define PIN_COUNT 8
#define NUMS 17

int pins[PIN_COUNT] = { 2, 3, 4, 5, 6, 7, 8, 9 };

void ClearPinMap() {
    printf("\ngood-bye\n");
    for (int b = 0; b < PIN_COUNT; ++b) {
        digitalWrite(pins[b], LOW);
    }
    exit(0);
}

int main() {
    signal(SIGINT, ClearPinMap);

    wiringPiSetupGpio();

    int sevenseq[NUMS][PIN_COUNT] = {
         {1, 1, 1, 1, 1, 1, 0, 0}   //0
        ,{0, 1, 1, 0, 0, 0, 0, 0}   //1
        ,{1, 1, 0, 1, 1, 0, 1, 0}   //2
        ,{1, 1, 1, 1, 0, 0, 1, 0}   //3
        ,{0, 1, 1, 0, 0, 1, 1, 0}   //4
        ,{1, 0, 1, 1, 0, 1, 1, 0}   //5
        ,{1, 0, 1, 1, 1, 1, 1, 0}   //6
        ,{1, 1, 1, 0, 0, 1, 0, 0}   //7
        ,{1, 1, 1, 1, 1, 1, 1, 0}   //8
        ,{1, 1, 1, 1, 0, 1, 1, 0}   //9
        ,{1, 1, 1, 0, 1, 1, 1, 0}   //A
        ,{0, 0, 1, 1, 1, 1, 1, 0}   //B
        ,{1, 0, 0, 1, 1, 1, 0, 0}   //C
        ,{0, 1, 1, 1, 1, 0, 1, 0}   //D
        ,{1, 0, 0, 1, 1, 1, 1, 0}   //E
        ,{1, 0, 0, 0, 1, 1, 1, 0}   //F
        ,{1, 1, 1, 1, 1, 1, 1, 1}   //X
    };

    for (int i = 0; i < PIN_COUNT; ++i)
        pinMode(pins[i], OUTPUT);

    while (1) {
        delay(1);
        for (int a = 0; a < NUMS; ++a) {
            printf("num:%d\n", a);
            for (int b = 0; b < PIN_COUNT; ++b) {
                digitalWrite(pins[b], sevenseq[a][b]);
            }
            delay(1000);
        }
    }
    return 0;
}