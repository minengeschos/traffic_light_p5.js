유튜브 영상
https://youtu.be/YhZ08HFGAOI


회로도
[https://www.tinkercad.com/things/4AYjenSZ41s/editel](https://www.tinkercad.com/things/4AYjenSZ41s/editel)
![image](https://github.com/user-attachments/assets/20d54288-bbd4-49b0-b184-101b9ee67b4d)

📌 사용된 핀 및 역할
핀 번호	역할	설명
LED 핀		
5번	LED_RED	빨간불
6번	LED_YELLOW	노란불
7번	LED_GREEN	초록불
버튼 핀		
2번	BUTTON_EMERGENCY	비상 정지 버튼 (긴급 모드 전환)
3번	BUTTON_RESET	시스템 ON/OFF 버튼
4번	BUTTON_BLINKING	전체 깜빡임 버튼
아날로그 입력		
A0	가변저항	LED 밝기 조절
🔄 작동 방식
기본적인 신호등 작동

빨간불 → 노란불 → 초록불 순서로 점등
각각의 신호는 설정된 ms 동안 유지됨
비상 모드 (BUTTON_EMERGENCY)

버튼을 누르면 비상 모드 (Emergency Mode) 로 전환
빨간불만 켜지고, 나머지 LED는 꺼짐
시스템 ON/OFF (BUTTON_RESET)

버튼을 누를 때마다 시스템을 켜거나 끌 수 있음
시스템이 꺼지면 모든 LED가 꺼짐
전체 깜빡임 (BUTTON_BLINKING)

버튼을 누르면 모든 LED가 동시에 깜빡이는 모드로 변경
0.5초 간격으로 깜빡임
가변저항 (A0)

가변저항 값을 읽어 LED 밝기를 조절
0~1023 값을 받아 0~255 사이로 변환하여 LED 밝기 조절
