#include <stdio.h>
#include <wiringPi.h>

#define LED_PIN 18                // LED의 GPIO 핀 번호
#define SWITCH_MODE 23            // 모드 전환 스위치의 GPIO 핀 번호
#define ENCODER_CLK 20            // 로터리 인코더 CLK 핀
#define ENCODER_DT 21             // 로터리 인코더 DT 핀
#define ENCODER_SW 17             // 로터리 인코더 선택 버튼 핀
#define SEG_A 2                   // 세븐 세그먼트 A 세그먼트 핀
#define SEG_B 3                   // 세븐 세그먼트 B 세그먼트 핀
#define SEG_C 4                   
#define SEG_D 5   
#define SEG_E 6                   
#define SEG_F 7   
#define SEG_G 8                   
#define SEG_H 9   

#define NUM_SEGS 7                // 세븐 세그먼트 세그먼트 개수
int segPins[NUM_SEGS] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_H};

int levels[] = {0, 100, 200};     // 밝기의 3 단계
int currentMode = 0;              // 현재 모드
int modeSetting = 0;              // 현재 모드의 세부 설정
volatile int prevEncoderClkState; // 이전 로터리 인코더 CLK 상태
volatile int prevEncoderSwState = HIGH; // 로터리 인코더 선택 버튼의 이전 상태
volatile int prevSwitchModeState = HIGH; // 모드 전환 스위치의 이전 상태

int sevenSeg[17][NUM_SEGS] = {
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

void setBrightness(int level) {
    pwmWrite(LED_PIN, levels[level]);
    printf("Current Brightness Level: %d\n", levels[level]);
}

void selectMode(int mode, int setting) {
    printf("Mode: %d, Setting: %d\n", mode, setting);
    // 서버에 모드와 세부 설정 정보를 전송하는 코드 (필요한 경우)
}

void updateModeSetting() {
    int currentClkState = digitalRead(ENCODER_CLK);
    int currentDtState = digitalRead(ENCODER_DT);

    if (currentClkState != prevEncoderClkState) {
        if (currentDtState != currentClkState) {
            modeSetting++;
        } else {
            modeSetting--;
        }

        // modeSetting 값을 0에서 16(포함하지 않음) 사이로 제한
        if (modeSetting >= 17) {
            modeSetting = 0;
        } else if (modeSetting < 0) {
            modeSetting = 16;
        }

        printf("Current Setting: %d\n", modeSetting);
        displaySevenSegment(modeSetting); // 세븐 세그먼트 디스플레이 업데이트
    }
    prevEncoderClkState = currentClkState;
}

void setup() {
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
}

void displaySevenSegment(int num) {
    for (int i = 0; i < NUM_SEGS; ++i) {
        digitalWrite(segPins[i], sevenSeg[num][i]);
    }
}

void loop() {
    int currentEncoderSwState = digitalRead(ENCODER_SW);
    int currentSwitchModeState = digitalRead(SWITCH_MODE);

    if (prevEncoderSwState == HIGH && currentEncoderSwState == LOW) {
        selectMode(currentMode, modeSetting);
    }
    prevEncoderSwState = currentEncoderSwState;

    if (prevSwitchModeState == HIGH && currentSwitchModeState == LOW) {
        currentMode = (currentMode + 1) % 3;
        setBrightness(currentMode);
        printf("Mode changed to: %d\n", currentMode);
    }
    prevSwitchModeState = currentSwitchModeState;

    updateModeSetting();
    delay(10);
}

int main() {
    setup();
    while (1) {
        loop();
    }
    return 0;
}
