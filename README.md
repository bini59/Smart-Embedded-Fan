
# Smart-Embedded-Fan 
금오공과대학교 임베디드시스템 기말프로젝트
바람을 넘어 기술로: 스마트 선풍기

## 💡 아이디어 소개
> ### 🎮 리모컨 제어 및 상태 표시
- 라즈베리 파이와 라즈베리 파이 간의 TCP 통신을 이용한다.
- 선풍기의 전원, 바람 세기 조절, 타이머 조작, 자동 모드 설정과 같은 주요 기능을 조작한다.
- 바람 세기, 자동 모드, 타이머 조작 상태를 LED와 세븐 세그먼트 디스플레이로 표시한다.

> ### 📱 스마트폰 제어
- 리모컨 분실 문제를 해결하기 위해 UART 블루투스 통신 기능을 사용한다.
- 선풍기의 전원, 바람 세기 조절, 타이머 조작, 자동 모드 설정과 같은 주요 기능을 조작한다.

> ### 🧑‍💻 웹 기반 제어
- 거리에 따른 조작 한계를 해결하기 위해 Node.js를 이용하여 Web 기반 제어 기능을 구현하였다.
- 바람 세기, 자동 모드, 타이머 조작 상태를 Web을 통해 확인할 수 있다.
- 선풍기의 전원, 바람 세기 조절, 타이머 조작, 자동 모드 설정과 같은 주요 기능을 조작한다.

> ### 🎥 객체 인식 및 추적
- 펜틸트 센서와 Caffe 프레임워크를 사용하여 사람 얼굴을 추적한다.
- 회전 자동 모드 설정 시 실행된다.
- 바람 방향을 사용자로 일정하게 타켓팅하여 추적한다.

> ### 📏 거리 기반 바람 제어
- 초음파 센서를 사용하여 사용자와 선풍기 간 거리를 계산한다.
- 바람 세기 자동 모드 설정 시 실행된다.
- 사용자와 선풍기 간 거리에 비례하여 바람 세기를 조절할 수 있다.

> ### 🌡️ 온습도 기반 바람 제어
- 온습도 센서를 사용하여 온습도를 수치로 계산한다.
- 바람 세기 자동 모드 설정 시 실행된다.
- 테스트에 따라 수치를 조정하여 온습도에 비례하여 바람 세기를 자동으로 조절할 수 있다.

> ### ⏰ 타이머 기능
- 선풍기에 내장된 타이머 기능을 활용하여 사용자가 원하는 시간에 선풍기가 자동으로 종료되도록 설정할 수 있다.
- 타이머 모드 설정 시 실행된다.

## 🔧 전체 시스템 구조도 
![image](https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/be3e6994-ef94-46d0-8e4b-818f452c1ad2)

## 🗺️ 회로구조도
![image](https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/b432af97-8003-47c7-a9f9-a7141962f961)

## 제한 조건 구현 내용 

> Thread - Mutex (유빈)
>
> 멀프 - IPC
>
> 멀프(유빈)
>
> IPC(승재)

## 가산점 요소

> Raspberry-Raspberry (제용)
> 
> Raspberry-SmartPhone (승재)
> 
> Raspberry-PC(Web) (유빈)

## 추가 기술 활용 : 얼굴 인식 기술을 활용한 회전 자동 모드의 구현

- 신경망 기반의 탐지기와 팬-틸트 메커니즘을 통해 얼굴을 정확하고 효율적으로 감지하고, 카메라 방향을 동적으로 조절한다. 
- 이 기능은 사용자의 얼굴을 화면 중앙에 유지하며, 실시간 처리 기능을 통해 원활하고 빠른 반응을 제공한다.
- 특히, 멀티 쓰레딩과 소켓 기반 서버를 활용한 원격 제어 기능이 핵심이다.

### 신경망 기반 얼굴 탐지

```python
# Caffe 모델을 사용한 신경망 기반 얼굴 탐지
detector = cv2.dnn.readNetFromCaffe("deploy.prototxt.txt", "res10_300x300_ssd_iter_140000.caffemodel")
frame = cv2.resize(frame, (300, 300))
blob = cv2.dnn.blobFromImage(frame, 1.0, (300, 300), (104.0, 177.0, 123.0))
detector.setInput(blob)
detections = detector.forward()
```

- OpenCV의 DNN 모듈을 사용하여 Caffe 프레임워크에서 훈련된 신경망 모델을 로드한다. 이 모델은 카메라에서 캡처된 프레임을 처리하여 얼굴을 감지하는 데 사용된다.

### 팬-틸트 메커니즘 제어

```python
# 팬(수평 회전)과 틸트(수직 회전) 각도 계산 및 조절
tiltAngle = (objX * 0.2) + move_x
panAngle = (objY * 0.2) + move_y
if in_range(tiltAngle, servoRange[0], servoRange[1]):
    pth.tilt(tiltAngle)
if in_range(panAngle, servoRange[0], servoRange[1]):
    pth.pan(panAngle)
```

- 감지된 얼굴의 위치를 기반으로 팬과 틸트의 각도를 계산한다. 이를 통해 카메라가 사용자의 얼굴을 화면 중앙에 유지하도록 조절한다.

### 멀티 쓰레딩 기술 활용

```python
# 얼굴 탐지 및 서보 제어 쓰레드 시작
processObjectCenter = threading.Thread(target=obj_center, args=(args,))
processObjectCenter.start()

# 팬-틸트 제어 쓰레드 시작
servoControl = threading.Thread(target=set_servos)
servoControl.start()
```

- 멀티 쓰레딩을 이용하여 얼굴 탐지, 팬-틸트 메커니즘 제어, 소켓 서버 통신 등을 동시에 수행한다. 이를 통해 각 기능이 서로 방해받지 않고 효율적으로 동작하며, 시스템의 전체적인 반응 속도와 안정성을 향상시킨다.

### 소켓 기반 서버를 통한 원격 제어

```python
# 소켓 서버 초기화 및 설정
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))
server_socket.listen(1)
```

- 서버 소켓을 초기화하여 특정 포트에서 연결 요청을 기다린다. 클라이언트로부터의 명령을 통해 시스템을 제어한다.

### 자동 모드 실행과 중지

```python
if data == '1':
    obj_tracking_running = True
    # 쓰레드 시작 로직
elif data == '0':
    obj_tracking_running = False
    # 팬-틸트 메커니즘 초기화 및 얼굴 탐지 기능 중지 로직
```

- 클라이언트로부터 '1' 명령을 받으면 자동 회전 모드를 시작한다. 이 모드에서는 얼굴 탐지와 팬-틸트 조절이 활성화되어 사용자의 얼굴을 화면 중앙에 유지한다.
- 클라이언트로부터 '0' 명령을 받으면 자동 회전 모드를 중지한다. 이때, 팬-틸트 메커니즘은 초기 위치로 돌아가고, 얼굴 탐지 기능도 비활성화된다.



## 실행 방법

### Remote controller
라즈베리파이-라즈베리파이 간의 TCP 연결을 위한 컨트롤러 
> 컴파일
```bash
$ gcc -o remote_client remote_client.c -lwiringPi # 리모컨 클라이언트
```

> 실행
```bash
$ sudo ./remote_client # 리모컨 클라이언트 실행
```
### Service 1
모든 컨트롤러의 요청을 받는 서버
> 컴파일
```bash
$ gcc -o main_server main_server.c -lrt -lwiringPi -lpthread  # 메인 서버
$ gcc -o dht_sensor dht_sensor.c -lrt -lwiringPi - lpthread # 온습도 센서
$ gcc -o ultrasonic_sensor ultrasonic_sensor.c -lrt -lwiringPi -lpthread # 초음파 센서
$ gcc -o bluetooth bluetooth.c -lrt -wiringPi # 블루투스 UART통신 센서
$ gcc -o server_exec server_exec.c -lrt # Web 통신 서버
$ gcc -o TCP_controlelr_connect TCP_controller_connect.c -lrt # Controller와 연결하기 위한 서버
$ gcc -o TCP_rotate_connect TCP_rotate_connect.c -lrt -lwiringPi # Service2 와 연결하기 위한 서버
```

> 실행
```bash
$ sudo ./main_server # 메인서버 실행
$ sudo ./bluetooth # UART 통신 서버 실행
$ sudo ./TCP_controlelr_connect # Controller 와 통신서버 실행
$ sudo ./TCP_rotate_connect # Service2 와 연결하기 위한 서버 실행
```

> node server 실행
```bash
$ cd service_1/node-server/
$ sudo node app.js
```

### Service 2
Service1으로 부터 회전 자동 모드 요청을 받는 서버
> 실행
```bash
$ python3 rotate_auto_server.py # 회전자동 모드 서버 실행
```

<br/>

## 조작 방법
- 사용자는 `Controller` `SmartPhone` `Web`을 통해 값을 Server에게 전달할 수 있다.
- 통신은 `Mode` + `Amount` 로 값을 Server에게 전달한다.
- Mode에는 `P`, `R`, `T` 가 있다.
  - `Power Mode(P)`  : 파워모드로 세기를 조절할 수 있다.
    - Amount : 0, 1, 2 , A  
  - `Rotate Mode(R)` : 회전모드로 회전을 조절할 수 있다.
    - Amount : 0 , 1 , A
  - `Timer Mode(T)`  : 타이머모드로 종료시간을 조절할 수 있다.
    - Amount : 0-9
- 예시
  - 파워 모드 2 설정: `P2`
  - 회전 모드 Auto 설정: `RA`
  - 타이머 모드 5 설정: `T5`



## 프로젝트 일정
<br/>

![image](https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/e05f2742-e023-47d1-99c6-19890a1c61de)
## 최종 결과물
<img src="https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/8cae6907-80c1-4009-863d-0e8d9feb3906" width="500" height="700"/>
<img src="https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/ef432f61-e350-4e1f-9c26-0bf249833d67" width="500" height="700"/>

## 시연 연상

<a href="https://www.youtube.com/watch?v=0BBzzAZkF_s">
    <img src="http://img.youtube.com/vi/0BBzzAZkF_s/0.jpg" width="100%" />
</a>

> 이미지를 클릭하면 유튜브로 이동

## 팀 명단
| Profile | Role | Part |
| ------- | ---- | ---- |
| <div align="center"><a href="https://github.com/..."><img src="..." width="70px;" alt=""/><br/><sub><b>송제용</b><sub></a></div> | 팀장 | ... |
| <div align="center"><a href="https://github.com/..."><img src="..." width="70px;" alt=""/><br/><sub><b>임유빈</b></sub></a></div> | 팀원 | ... |
| <div align="center"><a href="https://github.com/Sonny-Kor"><img src="..." width="70px;" alt=""/><br/><sub><b>손승재</b></sub></a></div> | 팀원 | ...| 
| <div align="center"><a href="https://github.com/..."><img src="..." width="70px;" alt=""/><br/><sub><b>박성현</b></sub></a></div> | 팀원 | ... | 
