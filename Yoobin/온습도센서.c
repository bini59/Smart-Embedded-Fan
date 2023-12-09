#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <wiringPi.h>

#define MAX_TIMINGS 85
#define DHT_PIN 26
int data[5] = {0, 0, 0, 0, 0};

void read_dht_data() {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    pinMode(DHT_PIN, OUTPUT);
    digitalWrite(DHT_PIN, LOW);
    delay(18);
    pinMode(DHT_PIN, INPUT);

    for (i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (digitalRead(DHT_PIN) == laststate) {
            counter++;
            delayMicroseconds(1);
            if (counter == 255) {
                break;
            }
        }
        laststate = digitalRead(DHT_PIN);

        if (counter == 255) break;

        if ((i >= 4) && (i % 2 == 0)) {
            data[j / 8] <<= 1;
            if (counter > 16)
                data[j / 8] |= 1;
            j++;
        }
    }
}

int main() {
    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "WiringPi 초기화 실패\n");
        exit(EXIT_FAILURE);
    }

    mqd_t mq;
    mq = mq_open("/posix_mq", O_WRONLY);
    char msg[128];

    while (1) {
        read_dht_data();
        if ((data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) && 
            (data[0] != 0 || data[2] != 0)) {
            sprintf(msg, "Humidity: %d.%d %% Temperature: %d.%d C", 
                data[0], data[1], data[2], data[3]);
            mq_send(mq, msg, strlen(msg), 0);
        } else {
            fprintf(stderr, "Invalid data\n");
        }
        usleep(2000000); // 2초 대기
    }

    mq_close(mq);
    return 0;
}
