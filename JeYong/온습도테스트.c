#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_TIMINGS 85
#define DHT_PIN 26
#define DELAY_MS 100  // 데이터 읽기 사이의 지연 시간 

int data[5] = {0, 0, 0, 0, 0};

void read_dht_data() {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;
    int attempts = 5; // 재시도 횟수

    while (attempts) {
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

        // 데이터 유효성 체크
        if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) &&
            (data[0] != 0 || data[2] != 0)) {
            printf("Humidity = %d.%d %% Temperature = %d.%d C\n",
                data[0], data[1], data[2], data[3]);
            return; // 성공적으로 데이터를 읽으면 함수 종료
        } else {
            printf("Data not good, skip\n");
            attempts--;
            delay(DELAY_MS); // 재시도 전에 지연 시간 추가
        }
    }
}

int main(void) {
    printf("Raspberry Pi DHT11 temperature/humidity test\n");

    if (wiringPiSetupGpio() == -1)
        exit(1);

    while (1) {
        read_dht_data();
        delay(DELAY_MS); // 데이터 읽기 사이에 지연 시간 추가
    }

    return 0;
}