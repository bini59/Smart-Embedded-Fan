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
#include <pthread.h>
#include <sys/time.h>

#include <stdint.h>

#define IN_A 18
#define IN_B 19
#define AUTO 17

#define DELAY_MS 1000  // 데이터 읽기 사이의 지연 시간 
int data[5] = {0, 0, 0, 0, 0};
int pin_arr[4] = {12, 16, 20, 21};
int one_two_phase[8][4] = {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,1,1}, {0,0,0,1},{1,0,0,1}};
const char* name = "/posix_mq";
char mode;
int amount;

pthread_t rotate_thread;
pthread_t tid_dht;
pthread_mutex_t lock;
pthread_mutex_t lock_d;
bool rotate_thread_running = false;
float sharedDistance = 0.0;

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
    printf("Roate Auto mode is on!!\n");
    while (rotate_thread_running) {
        one_two_Phase_Rotate_angle(130, 1); // 130도 회전
        one_two_Phase_Rotate_angle(130, 0); // 0도로 회전
    }
    
    return NULL;
}

void fork_ultrasonic_sensor() {
    pid_t pid = fork();
    if (pid == 0) {
        // 초음파 센서 프로그램 실행
        execlp("./ultrasonic_sensor", "ultrasonic_sensor", NULL);
        exit(EXIT_FAILURE);
    }
}

void fork_dht_sensor() {
    pid_t pid = fork();
    if (pid == 0) {
        // DHT 센서 프로그램 실행
        execlp("./dht_sensor", "dht_sensor", NULL);
        exit(EXIT_FAILURE);
    }
}

void *read_mq_data(void *arg) {
    mqd_t mq;
    struct mq_attr attr;
    char buffer[BUFSIZ];
    int n;

    // 메시지 큐 속성 설정
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = BUFSIZ;
    attr.mq_curmsgs = 0;

    // 메시지 큐 열기
    mq = mq_open("/posix_mq", O_RDONLY | O_CREAT, 0644, &attr);
    if (mq == -1) {
        perror("메시지 큐 열기 실패");
        return NULL;
    }

    while (1) {
        // 메시지 큐로부터 데이터 읽기
        n = mq_receive(mq, buffer, BUFSIZ, NULL);
        if (n == -1) {
            perror("메시지 큐 읽기 실패");
            continue;
        }

        buffer[n] = '\0'; // 문자열 종료 지정

        // 수신된 데이터 처리
        printf("Received: %s\n", buffer);

        // 여기서 센서 데이터에 따른 추가적인 처리를 구현할 수 있습니다.
        // 예: 버퍼에서 센서 데이터를 파싱하고 이를 기반으로 특정 작업 수행
    }

    mq_close(mq);
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

    // 초음파 센서, DHT 센서 프로그램 실행
    fork_ultrasonic_sensor();
    fork_dht_sensor();

    // 메시지 큐 읽기 스레드 생성
    pthread_create(&read_thread, NULL, read_mq_data, NULL);
    
    mq = mq_open(name, O_CREAT | O_RDONLY, 0644, &attr);
    while(1) {
        n = mq_receive(mq, buf, sizeof(buf), NULL);
        buf[n] = '\0';
        mode = buf[0];
        amount = buf[1] - '0';
        printf("%c %d\n",mode,amount);
        if(isOutOfRange(mode, amount)) {
            continue; // 범위를 벗어난 경우 반복 건너뛰기
        }

        // rotate는 스레드로 실행
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
    return 0;
}
