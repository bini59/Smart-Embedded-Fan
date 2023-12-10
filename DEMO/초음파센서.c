#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <wiringPi.h>
#include <sys/time.h>
#include <string.h>
#define TRIG 24
#define ECHO 23

long getMicrotime(){
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

float measureDistance() {
    long start, stop;
    float distance;

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    while(digitalRead(ECHO) == LOW);

    start = getMicrotime();
    while(digitalRead(ECHO) == HIGH);
    stop = getMicrotime();

    distance = (float)(stop - start) * 0.034029 / 2;
    return distance;
}

int main() {
    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "WiringPi 초기화 실패\n");
        exit(EXIT_FAILURE);
    }

    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);

    digitalWrite(TRIG, LOW);
    delay(30);

    mqd_t mq;
    mq = mq_open("/posix_sensor", O_WRONLY);
    char msg[128];

    while (1) {
        float distance = measureDistance();
        // printf("%f\n", distance);
        sprintf(msg, "D %f", distance);
        mq_send(mq, msg, strlen(msg), 0);
        usleep(500000); // 0.5초 대기
    }

    mq_close(mq);
    return 0;
}
