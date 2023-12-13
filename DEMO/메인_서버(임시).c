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
#include <stdint.h>

#define IN_A 18
#define IN_B 19
#define MINUTE 600 // 10분단위로 하기 ,, T1 -> 10분 타이머
#define TEST_TIME 10 // 테스트용 10초 ,, T1 -> 10초 타이머
#define AUTO 17

int data[5] = {0, 0, 0, 0, 0};
int pin_arr[4] = {12, 16, 20, 21};
int one_two_phase[8][4] = {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,1,1}, {0,0,0,1},{1,0,0,1}};
const char* name = "/posix_mq";
char mode;
int amount;
float distance = -1;
float humidity = -1;
float temperature = -1;
int timer_duration = 0;  

int rotate_value = 0;
int power_value = 0;
int timer_value = 0;

pthread_t rotate_thread;
pthread_t receive_thread;
pthread_t motor_thread;
pthread_t timer_thread;

pthread_mutex_t lock;
pthread_mutex_t lock_receive;

bool timer_thread_running = false;
bool rotate_thread_running = false;
bool motor_thread_running = false;
float sharedDistance = 0.0;

bool motor_thread_flag = false;



void save_command() {
    FILE *fp = fopen("data", "w");
    fprintf(fp, "%d\n%d\n%d", power_value, rotate_value, timer_value);
    fclose(fp);
}


void auto_rotate(bool flag){
    char buffer[BUFSIZ];
    if(flag)
        sprintf(buffer, "%c", '1');
    else
        sprintf(buffer, "%c", '0');
    mqd_t mq_R;
    const char* mq_name = "/posix_RA";
    mq_R = mq_open(mq_name, O_WRONLY);
    if(mq_R == (mqd_t)-1) {
        perror("mq_open");
        return;
        // exit(EXIT_FAILURE);
    }
    // 메시지 큐로 데이터 전송
    if(mq_send(mq_R, buffer, strlen(buffer), 0) == -1) {
        perror("mq_send");
    }
    // 메시지 큐 닫기
    mq_close(mq_R);
}
void setMotor(int speed, int direction) {
    if (direction == 1) {
        digitalWrite(IN_A, HIGH);
        digitalWrite(IN_B, LOW);
    } else {
        digitalWrite(IN_A, LOW);
        digitalWrite(IN_B, HIGH);
    }

    if (direction == 1) {
    pwmWrite(IN_A, speed);
    }
    else {
        pwmWrite(IN_A, 1024-speed);
    }
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
            return (amount < 0 || amount > 3);
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

    for(int i = 0; i< 4; i++){
        pinMode(pin_arr[i], OUTPUT);
    }

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

void *timer_func(){
    timer_thread_running = true;
    int elapsed_time = 0;
    printf("timer ON : %d\n", timer_duration);
    while (elapsed_time < timer_duration *  TEST_TIME && timer_thread_running) {
        usleep(1000000);  // 1초 대기
        elapsed_time++;
        
    }

    // PA 인 경우 -> P0
    if (motor_thread_running) {
        printf("thread is terminated!\n");
        motor_thread_running = false;
        motor_thread_flag = false;
        pthread_join(motor_thread, NULL);
    }
    // Rotate Mode OFF
    if (rotate_thread_running) {
        rotate_thread_running = false;
        pthread_join(rotate_thread, NULL);
    }
    timer_value = 0;
    power_value = 0;
    setMotor(0, 0);
    auto_rotate(false);

    save_command();

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

void *recv_sensor_data() {
    mqd_t mq;
    struct mq_attr attr;
    char buf[BUFSIZ];
    int str_len;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = BUFSIZ;
    attr.mq_curmsgs = 0;

    mq = mq_open("/posix_sensor", O_CREAT | O_RDONLY, 0644, &attr);
        if (mq == -1) {
        perror("메시지 큐 열기 실패");
        return NULL;
    }

    while(1) {
        str_len = mq_receive(mq, buf, sizeof(buf), NULL);
        if (str_len == -1) {
            perror("메시지 큐 읽기 실패");
            continue;
        }
        // printf("buf : %s\nbuf[0] : %c", buf, buf[0]);

        pthread_mutex_lock(&lock_receive);
        buf[str_len] = '\0';
        if (buf[0] == 'D') {
            distance = atof(buf + 2);
        } 
        else if (buf[0] == 'H') {
            // printf("%s\n", buf);
            sscanf(buf + 2, "%f %f", &humidity, &temperature);
        }
        pthread_mutex_unlock(&lock_receive);
        usleep(1000); // 1초 대기
    }

    mq_close(mq);
    return NULL;
}

int calculate_step(float distance, float humidity, float temperature) {
    int step_temp = 0, step_hum = 0, step_dist = 0;

    // Temperature conditions
    if (temperature >= 15 && temperature < 20)
        step_temp = 1;
    else if (temperature >= 20 && temperature < 25)
        step_temp = 2;
    else if (temperature >= 25 && temperature < 30)
        step_temp = 3;
    else if (temperature >= 30 && temperature < 35)
        step_temp = 4;
    else if (temperature >= 35)
        return 12;

    // Humidity conditions
    if (humidity >= 20 && humidity < 30)
        step_hum = 1;
    else if (humidity >= 30 && humidity < 40)
        step_hum = 2;
    else if (humidity >= 40 && humidity < 50)
        step_hum = 3;
    else if (humidity >= 50 && humidity < 60)
        step_hum = 4;
    else if (humidity >= 60)
        return 12;

    // Distance conditions
    if (distance >= 10 && distance < 30)
        step_dist = 1;
    else if (distance >= 30 && distance < 50)
        step_dist = 2;
    else if (distance >= 50 && distance < 70)
        step_dist = 3;
    else if (distance >= 70 && distance < 90)
        step_dist = 4;
    else if (distance >= 90)
        return 12;

    // printf("step : %d %d %d\n", step_dist, step_hum, step_temp);
    int total = step_dist + step_hum + step_temp;
    return total;
}

void *run_motor() {
    int step = 0;

    while(motor_thread_flag) {
        if (distance < -1 || humidity < -1 || temperature < -1) continue;

        step = calculate_step(distance, humidity, temperature);
        if (distance < 10 || temperature < 15 || humidity < 20) step = 0;
        setMotor(step*50, 0);       
        // printf("total step : %d\n", step);

        usleep(100000); // 0.1초 대기

    }

    return NULL;
}

void command_timer() {
    if (timer_thread_running) {
        // 기존 타이머가 실행 중이면 중지
        timer_thread_running = false;
        pthread_join(timer_thread, NULL);
    }
    timer_duration = amount;
    pthread_create(&timer_thread, NULL, timer_func, NULL);
}

void command_rotate() {
    if (amount == 1) {
        auto_rotate(false);
        if (!rotate_thread_running) {
            pthread_create(&rotate_thread, NULL, rotate, NULL);
        }
    } else if(amount == 0) {
        auto_rotate(false);
        if (rotate_thread_running) {
            pthread_mutex_lock(&lock);
            rotate_thread_running = false;
            pthread_mutex_unlock(&lock);
            pthread_join(rotate_thread, NULL);
        }
    } else if(amount == AUTO){
        if (rotate_thread_running) {
            pthread_mutex_lock(&lock);
            rotate_thread_running = false;
            pthread_mutex_unlock(&lock);
            pthread_join(rotate_thread, NULL);
        }
        auto_rotate(true);

    }
}

void command_power() {
    printf("amount : %d\n", amount);
    if (amount < AUTO) {
        if (motor_thread_running) {
            printf("thread is terminated!\n");
            motor_thread_running = false;
            motor_thread_flag = false;
            pthread_join(motor_thread, NULL);
        }
    }


    if (amount == 0) {
        setMotor(0, 0);
        if (rotate_thread_running) {
            rotate_thread_running = false;
            pthread_join(rotate_thread, NULL);
        }
    } else if(amount == 1) {
        setMotor(200, 0);
    } else if(amount == 2) {
        setMotor(300, 0);
    } else if(amount == 3) {
        setMotor(400, 0);
    } else if(amount == AUTO) {
        if (!motor_thread_running) {
            motor_thread_flag = true;
            motor_thread_running = true;
            pthread_create(&motor_thread, NULL, run_motor, NULL);
        }
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

    init();

    // 초음파 센서, DHT 센서 프로그램 실행
    fork_ultrasonic_sensor();
    fork_dht_sensor();

    // 메시지 큐 읽기 스레드 생성
    pthread_create(&receive_thread, NULL, recv_sensor_data, NULL);
    
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

        if (mode == 'R') {
            rotate_value = amount;
            command_rotate();
        } else if(mode == 'P') {
            power_value = amount;
            command_power();
        } else if(mode == 'T') {
            timer_value = amount;
            command_timer();
        }
        save_command();
        usleep(100000); // 0.1초 대기
    }

    mq_close(mq);
    mq_unlink(name);
    return 0;
}
