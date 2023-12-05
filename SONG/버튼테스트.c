#include <stdio.h>
#include <wiringPi.h>

#define LED_PIN 18                // LED의 GPIO 핀 번호
#define SWITCH_MODE 23            // 모드 전환 스위치의 GPIO 핀 번호
#define ENCODER_CLK 20            // 로터리 인코더 CLK 핀
#define ENCODER_DT 21             // 로터리 인코더 DT 핀
#define ENCODER_SW 17             // 로터리 인코더 선택 버튼 핀

int brightness = 0;               // 현재 밝기 레벨
int levels[] = {0, 100, 200};     // 밝기의 3 단계
int currentMode = 0;              // 현재 모드
int modeSetting = 0;              // 현재 모드의 세부 설정
volatile int prevEncoderClkState; // 이전 로터리 인코더 CLK 상태
volatile int prevEncoderSwState = HIGH; // 로터리 인코더 선택 버튼의 이전 상태
volatile int prevSwitchModeState = HIGH; // 모드 전환 스위치의 이전 상태

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
        printf("Current Setting: %d\n", modeSetting);
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

    prevEncoderClkState = digitalRead(ENCODER_CLK);
    setBrightness(brightness);
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
