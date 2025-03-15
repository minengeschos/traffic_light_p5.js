#include <Arduino.h>
#include <TaskScheduler.h>

// LED 핀 정의
#define LED_RED     5   // 빨간 LED
#define LED_YELLOW  6   // 노란 LED
#define LED_GREEN   7   // 초록 LED
#define LED_BLINK   8   // 깜빡임 LED

// 버튼 핀 정의
#define BUTTON_EMERGENCY 2  // 비상 모드 버튼 (외부 인터럽트 INT0)
#define BUTTON_RESET     3  // 시스템 OFF/ON 버튼 (외부 인터럽트 INT1)
#define BUTTON_BLINKING  4  // 깜빡임 모드 버튼 (폴링 방식)

// 모드 플래그 (인터럽트 또는 폴링에 의해 변경됨)
volatile bool emergencyMode = false;  // 비상 모드
volatile bool systemOffMode = false;  // 시스템 OFF 모드
volatile bool blinkingMode  = false;  // 깜빡임 모드

// 디바운스 관련 변수 (인터럽트 방지)
const unsigned long debounceDelayMicro = 200000; // 200ms 디바운스 시간
volatile unsigned long lastEmergencyInterruptTime = 0;
volatile unsigned long lastResetInterruptTime     = 0;

// 깜빡임 모드 버튼 디바운스 (millis 단위)
unsigned long lastBlinkingDebounceTime = 0;
const unsigned long blinkingDebounceDelay = 50; // 50ms 디바운스 시간
int lastBlinkingButtonState = HIGH;  // 버튼의 이전 상태

// TaskScheduler 관련 변수
Scheduler runner;
const unsigned long cycleDuration = 6000;  // 신호등 주기 (6초)
unsigned long cycleStart = 0;

// 신호등 상태 업데이트 함수 선언
void updateTrafficLights();
Task trafficTask(100, TASK_FOREVER, &updateTrafficLights, &runner, true);

// 비상 모드 인터럽트 핸들러 (INT0 사용)
void emergencyISR() {
  unsigned long current = micros();
  if (current - lastEmergencyInterruptTime >= debounceDelayMicro) {
    emergencyMode = !emergencyMode;  // 모드 토글
    lastEmergencyInterruptTime = current;
  }
}

// 시스템 OFF/ON 모드 인터럽트 핸들러 (INT1 사용)
void resetISR() {
  unsigned long current = micros();
  if (current - lastResetInterruptTime >= debounceDelayMicro) {
    systemOffMode = !systemOffMode;  // 모드 토글
    lastResetInterruptTime = current;
  }
}

// 깜빡임 모드 버튼 폴링 방식 처리
void pollBlinkingButton() {
  int currentState = digitalRead(BUTTON_BLINKING);
  unsigned long currentTime = millis();
  if ((lastBlinkingButtonState == HIGH) && (currentState == LOW) &&
      (currentTime - lastBlinkingDebounceTime > blinkingDebounceDelay)) {
    blinkingMode = !blinkingMode;  // 모드 토글
    lastBlinkingDebounceTime = currentTime;
  }
  lastBlinkingButtonState = currentState;
}

// 현재 신호등 상태를 시리얼로 전송하는 함수
void sendTrafficLightStatus() {
  String statusMessage;

  // 1. 시스템 OFF 상태
  if (systemOffMode) {
    statusMessage = "STATUS: SYSTEM OFF";
  }
  // 2. 비상 모드
  else if (emergencyMode) {
    statusMessage = "STATUS: EMERGENCY MODE";
  }
  // 3. 깜빡임 모드
  else if (blinkingMode) {
    statusMessage = "STATUS: BLINKING MODE";
  }
  // 4. 정상 신호등 상태
  else {
    unsigned long now = millis();
    unsigned long elapsed = (now - cycleStart) % cycleDuration;
    
    if (elapsed < 2000) {
      statusMessage = "STATUS: RED LIGHT";
    } else if (elapsed < 2500) {
      statusMessage = "STATUS: YELLOW LIGHT";
    } else if (elapsed < 4500) {
      statusMessage = "STATUS: GREEN LIGHT";
    } else if (elapsed < 5500) {
      statusMessage = "STATUS: BLINKING LIGHT";
    } else {
      statusMessage = "STATUS: YELLOW LIGHT";
    }
  }

  // 시리얼로 상태 전송
  Serial.println(statusMessage);
}

// 아두이노 초기 설정 함수
void setup() {
  // 시리얼 통신 시작
  Serial.begin(9600);

  // LED 핀을 출력으로 설정
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLINK, OUTPUT);
  
  // 초기 LED OFF
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLINK, LOW);
  
  // 버튼 핀을 입력으로 설정 (내부 풀업 저항 사용)
  pinMode(BUTTON_EMERGENCY, INPUT_PULLUP);
  pinMode(BUTTON_RESET, INPUT_PULLUP);
  pinMode(BUTTON_BLINKING, INPUT_PULLUP);
  
  // 외부 인터럽트 등록
  attachInterrupt(digitalPinToInterrupt(BUTTON_EMERGENCY), emergencyISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RESET), resetISR, FALLING);
  
  // 신호등 동작 시작 시간 설정
  cycleStart = millis();
}

// 메인 루프 함수
void loop() {
  runner.execute();      // TaskScheduler 실행
  pollBlinkingButton();  // 깜빡임 버튼 폴링

  // 현재 신호등 상태를 시리얼로 전송
  sendTrafficLightStatus();
}

// 신호등 상태 업데이트 함수
void updateTrafficLights() {
  int brightness = map(analogRead(A0), 0, 1023, 0, 255);  // 가변저항 값 -> LED 밝기 매핑
  
  // 1. 시스템 OFF 모드: 모든 LED OFF
  if (systemOffMode) {
    analogWrite(LED_RED, 0);
    analogWrite(LED_YELLOW, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLINK, 0);
    return;
  }
  
  // 2. 비상 모드: 빨간 LED만 켜기
  if (emergencyMode) {
    analogWrite(LED_RED, brightness);
    analogWrite(LED_YELLOW, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLINK, 0);
    return;
  }
  
  // 3. 깜빡임 모드: 모든 LED가 0.5초 주기로 깜빡임
  if (blinkingMode) {
    bool blinkState = ((millis() / 500) % 2) == 0;
    analogWrite(LED_RED,     blinkState ? brightness : 0);
    analogWrite(LED_YELLOW,  blinkState ? brightness : 0);
    analogWrite(LED_GREEN,   blinkState ? brightness : 0);
    analogWrite(LED_BLINK,   blinkState ? brightness : 0);
    return;
  }
  
  // 4. 정상 신호등 동작 (6초 주기)
  unsigned long now = millis();
  unsigned long elapsed = (now - cycleStart) % cycleDuration;
  
  // 모든 LED OFF 후 현재 상태에 맞는 LED ON
  analogWrite(LED_RED, 0);
  analogWrite(LED_YELLOW, 0);
  analogWrite(LED_GREEN, 0);
  analogWrite(LED_BLINK, 0);
  
  if (elapsed < 2000) {
    analogWrite(LED_RED, brightness);  // 0~2초: 빨간불
  }
  else if (elapsed < 2500) {
    analogWrite(LED_YELLOW, brightness);  // 2~2.5초: 노란불
  }
  else if (elapsed < 4500) {
    analogWrite(LED_GREEN, brightness);  // 2.5~4.5초: 초록불
  }
  else if (elapsed < 5500) {
    int tick = (elapsed - 4500) / 100;
    if (tick == 0 || tick == 3 || tick == 6) {
      analogWrite(LED_BLINK, brightness);  // 4.5~5.5초: 깜빡이는 불
    }
  }
  else {
    analogWrite(LED_YELLOW, brightness);  // 5.5~6초: 노란불
  }
}
