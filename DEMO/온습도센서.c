#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <wiringPi.h>
#include <string.h>
#include <stdint.h>

#define MAX_TIMINGS 85
#define DHT_PIN 26
#define DELAY_MS 10  // 데이터 읽기 사이의 지연 시간 
#define TIMEOUT 10000 // 타임아웃을 위한 카운터 한계
int data[5] = {0, 0, 0, 0, 0};

float humidity;
float temperature;

// void read_dht_data() {
//     uint8_t laststate = HIGH;
//     uint8_t counter = 0;
//     uint8_t j = 0, i;

//     data[0] = data[1] = data[2] = data[3] = data[4] = 0;

//     pinMode(DHT_PIN, OUTPUT);
//     digitalWrite(DHT_PIN, LOW);
//     delay(18);
//     pinMode(DHT_PIN, INPUT);

//     for (i = 0; i < MAX_TIMINGS; i++) {
//         counter = 0;
//         while (digitalRead(DHT_PIN) == laststate) {
//             counter++;
//             delayMicroseconds(1);
//             if (counter == 255) {
//                 break;
//             }
//         }
//         laststate = digitalRead(DHT_PIN);

//         if (counter == 255) break;

//         if ((i >= 4) && (i % 2 == 0)) {
//             data[j / 8] <<= 1;
//             if (counter > 16)
//                 data[j / 8] |= 1;
//             j++;
//         }
//     }
// }


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

        uint32_t timeout_counter = 0; // 타임아웃 카운터 초기화

        for (i = 0; i < MAX_TIMINGS; i++) {
            counter = 0;
            while (digitalRead(DHT_PIN) == laststate) {
                counter++;
                delayMicroseconds(1);
                if (counter == 255 || ++timeout_counter > TIMEOUT) {
                    break;
                }
            }
            laststate = digitalRead(DHT_PIN);

            if (counter == 255 || timeout_counter > TIMEOUT) {
                break;
            }

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
            humidity = (float)data[0] + data[1] / 10.0;
            temperature = (float)data[2] + data[3] / 10.0;
            // printf("Humidity = %.1f %% Temperature = %.1f C\n", humidity, temperature);
            return; // 성공적으로 데이터를 읽으면 함수 종료
        } else {
            attempts--;
            delay(DELAY_MS); // 재시도 전에 지연 시간 추가
        }
    }
}

int main() {
    if (wiringPiSetupGpio() == -1) {
        fprintf(stderr, "WiringPi 초기화 실패\n");
        exit(EXIT_FAILURE);
    }

    mqd_t mq;
    mq = mq_open("/posix_sensor", O_WRONLY);
    char msg[128];

    while (1) {
        read_dht_data();
        if ((data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) && 
            (data[0] != 0 || data[2] != 0)) {
            sprintf(msg, "H %d.%d %d.%d", 
                data[0], data[1], data[2], data[3]);
            mq_send(mq, msg, strlen(msg), 0);
        } else {
            // fprintf(stderr, "Invalid data\n");
        }
        usleep(2000000); // 2초 대기
    }

    mq_close(mq);
    return 0;
}
