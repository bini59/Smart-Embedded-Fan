#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <wiringPi.h>
#include <signal.h>

#define PORT 8080

int sock;
struct sockaddr_in serv_addr;

#define LED_PIN 18
#define SWITCH_MODE 23
#define ENCODER_CLK 20
#define ENCODER_DT 21
#define ENCODER_SW 17
#define SEG_A 2
#define SEG_B 3
#define SEG_C 4
#define SEG_D 5
#define SEG_E 6
#define SEG_F 7
#define SEG_G 8
#define SEG_H 9
#define NUM_SEGS 8
int segPins[NUM_SEGS] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_H};
int levels[] = {0, 50, 250};
int currentMode = 0;
int modeSetting = 0;
volatile int prevEncoderClkState;
volatile int prevEncoderSwState = HIGH;
volatile int prevSwitchModeState = HIGH;

int sevenSeg[12][NUM_SEGS] = {
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
    ,{1, 1, 1, 0, 1, 1, 1, 0}   //A : 자동
    ,{0, 0, 0, 0, 0, 0, 0, 0}   //OFF
};

// 함수 선언
void updateSevenSegmentDisplay();
void displaySevenSegment(int num);

void setBrightness(int level) {
    pwmWrite(LED_PIN, levels[level]);
    printf("모드에 따른 밝기 값: %d\n", levels[level]);
}

void updateModeSetting() {
    int currentClkState = digitalRead(ENCODER_CLK);
    int currentDtState = digitalRead(ENCODER_DT);

    if (currentClkState != prevEncoderClkState) {
        if (currentDtState != currentClkState) {
            modeSetting++; // 시계 방향 회전
        } else {
            modeSetting--; // 반시계 방향 회전
        }

        // modeSetting 범위 조정
        if (modeSetting < 0) {
            modeSetting = 0;
        } else if (modeSetting > 39) {
            modeSetting = 39;
        }

        printf("로터리 값: %d\n", modeSetting);
        updateSevenSegmentDisplay(); // 세븐세그먼트 값 변경
    }

    prevEncoderClkState = currentClkState;
}

void updateSevenSegmentDisplay() {
    int segmentValue;
    if (currentMode == 0) {
        if (modeSetting < 10) segmentValue = 10; // A
        else if (modeSetting < 20) segmentValue = 0; // 0
        else if (modeSetting < 30) segmentValue = 1; // 1
        else segmentValue = 2; // 2
    } else if (currentMode == 1) {
        if (modeSetting < 13) segmentValue = 10; // A
        else if (modeSetting < 26) segmentValue = 0; // 0
        else segmentValue = 1; // 1
    } else if (currentMode == 2) {
        if (modeSetting < 4) segmentValue = 1; // 1
        else if (modeSetting < 8) segmentValue = 2; // 2
        else if (modeSetting < 12) segmentValue = 3; // 3
        else if (modeSetting < 16) segmentValue = 4; // 4
        else if (modeSetting < 20) segmentValue = 5; // 5
        else if (modeSetting < 24) segmentValue = 6; // 6
        else if (modeSetting < 28) segmentValue = 7; // 7
        else if (modeSetting < 32) segmentValue = 8; // 9
        else segmentValue = 9; // 9
    }

    printf("모드에 따른 세부 모드: %d\n", segmentValue);
    displaySevenSegment(segmentValue);
}

void displaySevenSegment(int num) {
    for (int i = 0; i < NUM_SEGS; ++i) {
        digitalWrite(segPins[i], sevenSeg[num][i]);
    }
}

void selectMode(int mode, int setting) {
    char prefix; //모드
    char segmentValue;  //세부 모드 

    // 접두어 설정
    switch (mode) {
        case 0: prefix = 'P'; break;
        case 1: prefix = 'R'; break;
        case 2: prefix = 'T'; break;
        default: prefix = 'X'; // 예외 처리
    }

    // 접미사 설정
    if (mode == 0) {
        if (setting < 10) segmentValue = 'A'; // 'A'
        else segmentValue = (setting < 20) ? '0' : ((setting < 30) ? '1' : '2'); // '0', '1', '2'
    } else if (mode == 1) {
        if (setting < 13) segmentValue = 'A'; // 'A'
        else segmentValue = (setting < 26) ? '0': '1'; // '1', '2'
    } else if (mode == 2) { // T 모드
        if (setting < 4) segmentValue = '1';
        else if (setting < 8) segmentValue = '2';
        else if (setting < 12) segmentValue = '3';
        else if (setting < 16) segmentValue = '4';
        else if (setting < 20) segmentValue = '5';
        else if (setting < 24) segmentValue = '6';
        else if (setting < 28) segmentValue = '7';
        else if (setting < 32) segmentValue = '8';
        else segmentValue = '9';
    }
    char message[50];
    sprintf(message, "%c%c", prefix, segmentValue);
    if (send(sock, message, strlen(message), 0) < 0) {
        printf("Failed to send message\n");
    } else {
        printf("Sent: %s\n", message);
    }
}

void cleanup() {
    displaySevenSegment(11); // 7_세그먼트 꺼짐
    pwmWrite(LED_PIN, 0); // LED 꺼짐
    printf("\n프로그램 종료\n");
    exit(0); // 프로그램 종료
}

void setup() {
     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "172.20.10.13", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }

    wiringPiSetupGpio();
    pinMode(LED_PIN, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(200);
    pwmSetClock(96);
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);
    pinMode(ENCODER_SW, INPUT);
    pinMode(SWITCH_MODE, INPUT);
    pullUpDnControl(ENCODER_SW, PUD_UP); // 내부 풀업 저항 활성화

    for (int i = 0; i < NUM_SEGS; ++i) {
        pinMode(segPins[i], OUTPUT);
    }

    prevEncoderClkState = digitalRead(ENCODER_CLK);
    setBrightness(currentMode);
    updateSevenSegmentDisplay(); //초기값으로 초기화
}


void loop() {
    int currentEncoderSwState = digitalRead(ENCODER_SW);
    int currentSwitchModeState = digitalRead(SWITCH_MODE);

    if (prevEncoderSwState == LOW && currentEncoderSwState == HIGH) {
        selectMode(currentMode, modeSetting);
    }
    prevEncoderSwState = currentEncoderSwState;

    if (prevSwitchModeState == LOW && currentSwitchModeState == HIGH) {
        currentMode = (currentMode + 1) % 3;
        setBrightness(currentMode);
        printf("모드 변경: %d\n", currentMode);
        modeSetting = 0; // 모드 변경 시 modeSetting 초기화
        updateSevenSegmentDisplay(); //세븐세그먼트도 초기화
    }
    prevSwitchModeState = currentSwitchModeState;

    updateModeSetting();
    delay(10);
}

int main() {
    if (wiringPiSetupGpio() == -1) {
        return 1;
    }
    signal(SIGINT, cleanup);
    setup();
    while (1) {
        loop();
    }
    cleanup();
    return 0;
}
