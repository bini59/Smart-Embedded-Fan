/*
컴파일시
gcc -o Server Server.c -lrt -lwiringPi -lpthread
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
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>


#define IN_A 18
#define IN_B 19
#define AUTO 17
#define TRIG 24
#define ECHO 23
#define DHT_PIN 26

#define MAX_TIMINGS 85
#define DELAY_MS 10000  // 데이터 읽기 사이의 지연 시간 
#define TIMEOUT 10000 // 타임아웃을 위한 카운터 한계

int data[5] = {0, 0, 0, 0, 0};
int pin_arr[4] = {12, 16, 20, 21};
int one_two_phase[8][4] = {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,1,1}, {0,0,0,1},{1,0,0,1}};
const char* name = "/posix_mq";
char mode; // Mode 값
int amount; // 1단 2단 결정하는 값

pthread_t dht_thread; // 온습도센서 쓰레드
pthread_t rotate_thread; // 회전모터 쓰레드
pthread_t fan_thread; // 선풍기 모터 쓰레드

pthread_mutex_t lock_dht; // 온습도 센서용 Lock
pthread_mutex_t lock_d; // 초음파 센서용 Lock
pthread_mutex_t lock; // 회전 센서용 Lock

bool rotate_thread_running = false; // 회전 모터 On/Off 용 변수
float sharedDistance = 0.0; // 전역 변수로 거리 저장
float humidity, temperature; // 전역 변수로 습도와 온도를 저장

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
            pthread_mutex_lock(&lock_dht);
            float tmp1 = (float)data[0] + data[1] / 10.0;
            float tmp2 = (float)data[2] + data[3] / 10.0;
            if(tmp1 < 100 || tmp1 > 0)
                humidity = tmp1;
            if(tmp2 < 50 || tmp2 > -20)
                temperature = tmp2;
            pthread_mutex_unlock(&lock_dht);
            printf("Humidity = %.1f %% Temperature = %.1f C\n", humidity, temperature);
            return; // 성공적으로 데이터를 읽으면 함수 종료
        } else {
            attempts--;
            delay(DELAY_MS); // 재시도 전에 지연 시간 추가
        }
    }
}
void* readTemperatrue_Humidity(){
    while (1) {
        read_dht_data();
    }
}

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
// 현재 시간을 마이크로초로 반환하는 함수
long getMicrotime(){
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}
// 거리를 측정하는 함수
float measureDistance() {
    long start, stop;
    float distance;

    // 초음파 신호를 10 마이크로초 동안 발생
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    // Echo 핀이 HIGH 상태가 될 때까지 기다림 (신호 발생 대기)
    while(digitalRead(ECHO) == LOW);

    start = getMicrotime(); // 신호가 도착하기 시작한 시간 기록
    // Echo 핀이 LOW 상태가 될 때까지 기다림 (신호 수신 완료)
    while(digitalRead(ECHO) == HIGH);
    stop = getMicrotime(); // 신호 수신 완료 시간 기록

    // 거리 계산 (시간 차이 * 음속 / 2) 및 mm 단위로 변환
    distance = (float)(stop - start) * 0.034029 / 2 * 10; // cm를 mm로 변환

    return distance;
}

bool isOutOfRange(char mode, int amount) {
    
    switch (mode) {
        case 'P':
            if(amount == AUTO)
                return false;
            return (amount < 0 || amount > 2);
        case 'R':
            if(amount == AUTO)
                return false;
            return (amount < 0 || amount > 1);
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
    pinMode(TRIG, OUTPUT); // Trig 핀을 출력으로 설정
    pinMode(ECHO, INPUT);  // Echo 핀을 입력으로 설정

    for(int i = 0; i< 4; i++){
        pinMode(pin_arr[i], OUTPUT);
    }

    // 초음파 센서 초기화
    digitalWrite(TRIG, LOW);
    delay(30);

}
void *distanceThread(void *arg) {
    while(1) {
        pthread_mutex_lock(&lock_d);
        sharedDistance = measureDistance();
        pthread_mutex_unlock(&lock_d);
        printf("distance thread is on!!\n");

        if(amount == AUTO && mode == 'P'){
            printf("Amount is Auto!!\n");
            if(sharedDistance > 1000) setMotor(300,1);
            else setMotor(sharedDistance*0.25, 1);
        }else if(mode == 'P'){
            setMotor(100*(amount), 1);                    
        }
        delay(200); // 5Hz 주파수 유지
    }
    return NULL;
}
int one_two_Phase_Rotate_angle(float angle, int dir){
    int steps = ((4048 / 360)) * angle;
    if(dir == 1){
        for(int i = 0; i<steps; i++){
            digitalWrite(pin_arr[0], one_two_phase[i%8][0]);  
            digitalWrite(pin_arr[1], one_two_phase[i%8][1]);
            digitalWrite(pin_arr[2], one_two_phase[i%8][2]);
            digitalWrite(pin_arr[3], one_two_phase[i%8][3]);
            delay(3);
        }
    }

    if(dir == 0){
        for(int i = 0; i<steps; i++){
            digitalWrite(pin_arr[0], one_two_phase[i%8][3]);
            digitalWrite(pin_arr[1], one_two_phase[i%8][2]);
            digitalWrite(pin_arr[2], one_two_phase[i%8][1]);
            digitalWrite(pin_arr[3], one_two_phase[i%8][0]);
            delay(3);
        }
    }
}
void *rotate(void *arg) {
    pthread_mutex_lock(&lock);
    rotate_thread_running = true;
    pthread_mutex_unlock(&lock);
    printf("Roate mode is on!!\n");
    while (rotate_thread_running) {
        one_two_Phase_Rotate_angle(130, 1); // 130도 회전
        one_two_Phase_Rotate_angle(130, 0); // 0도로 회전
    }
    
    return NULL;
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
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lock_d, NULL);
    pthread_create(&fan_thread, NULL, &distanceThread, NULL);
    pthread_create(&dht_thread, NULL, &readTemperatrue_Humidity, NULL);
    
    mq = mq_open(name, O_CREAT | O_RDONLY, 0644, &attr);
    while(1) {
        n = mq_receive(mq, buf, sizeof(buf), NULL);
        buf[n] = '\0';
        if(isOutOfRange( buf[0], buf[1] - '0')) {
            continue; // 범위를 벗어난 경우 반복 건너뛰기
        }
        mode = buf[0];
        amount = buf[1] - '0';
        printf("%c %d\n",mode,amount);
        
        if (mode == 'R' && amount == 1) {
            if (!rotate_thread_running) {
                pthread_create(&rotate_thread, NULL, rotate, NULL);
            }
        } else if(mode == 'R' && amount == 0) {
            if (rotate_thread_running) {
                rotate_thread_running = false;
                pthread_join(rotate_thread, NULL);
            }
        }
        usleep(100000); // 0.1초 대기
    }

    mq_close(mq);
    mq_unlink(name);
    pthread_join(fan_thread, NULL);
    pthread_join(dht_thread,NULL);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&lock_d);
    return 0;
}
