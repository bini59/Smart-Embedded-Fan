
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

## 🔒︎ 제한 조건 구현 내용 
![image](https://github.com/bini59/Smart-Embedded-Fan/assets/118044367/36d2237f-e4a0-4fe1-bd81-f78fc2bccd36)

> Thread - Mutex (유빈)
>
> 멀프 - IPC
>
> 멀프(유빈)
>
> IPC(승재)

## ❤️ 가산점 요소
### 1. RaspberryPi-RaspberryPi

- 리모컨 라즈베리파이
    - 사용자의 명령 입력을 받는 리모컨 기능을 수행한다. 사용자는 이 장치를 통해 선풍기의 전원, 바람 세기, 타이머 설정 등을 제어할 수 있다.
    - 사용자의 입력은 메인 서버로 전송되어, 해당 명령에 따라 선풍기가 작동하도록 한다.
- 회전자동모드(얼굴인식) 라즈베리파이
   - 얼굴 인식 기능을 위한 전용 서버로 활용된다. 메인 서버를 통해 작동 명령을 받으면 작동한다.
   - 카메라를 통해 사용자의 얼굴을 실시간으로 추적하고, 이 정보를 기반으로 팬(수평 회전)과 틸트(수직 회전) 각도를 조정한다.
- 제한 조건 구현 내용에 리모컨 라즈베리 파이에 대한 내용이 기술 되어 있으므로, 여기서는 회전자동모드를 위한 라즈베리 파이에 대한 구현을 상세히 설명한다.

> ### Client RaspberryPi (메인 서버)  - RA 값을 이용한 송신

```c
/* main_server.c */
void auto_rotate(bool flag){
    char buffer[BUFSIZ];
    if(flag)
        sprintf(buffer, "%c", '1'); // RA 입력 시, '1' 전송
    else
        sprintf(buffer, "%c", '0'); // 그 외 경우, '0' 전송
    // 메시지 큐를 통한 데이터 전송
    if (mq_send(mq_R, buffer, strlen(buffer), 0) == -1) {
        perror("mq_send");
    }
    mq_close(mq_R);
}
```

- **`auto_rotate`** 함수는 메인 서버에서 RA 명령을 수신받았을 때, 회전자동모드를 활성화하거나 비활성화하기 위해 '1' 또는 '0'을 회전자동모드 라즈베리 파이에 전송한다.

```c
/* TCP_rotate_connect.c */
// TCP 클라이언트 소켓 설정 및 연결
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    exit(EXIT_FAILURE);
}
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(PORT);
if(inet_pton(AF_INET, "172.20.10.2", &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    exit(EXIT_FAILURE);
}
if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed \n");
    exit(EXIT_FAILURE);
}

// 데이터 전송 로직
char message[50];
sprintf(message, "%c", '1'); // 예시 데이터 '1' 전송
if (send(sock, message, strlen(message), 0) < 0) {
    printf("Failed to send message\n");
} else {
    printf("Sent: %s\n", message);
}
```

- 메인 서버 라즈베리 파이가 회전자동모드 라즈베리 파이에 연결을 시도하는 클라이언트 소켓을 설정한다.
- **`send`** 함수를 사용하여 예시 데이터 ('1'이나 '0')를 서버에 전송한다. 이는 메인 서버에서 회전자동모드의 활성화 또는 비활성화를 제어하는 데 사용된다.

> ### Server RaspberryPi (회전 자동 모드)  - 소켓을 통한 수신 및 처리

```python
/* rotate_auto_server.py */
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))
server_socket.listen(1)

while True:
    client_socket, addr = server_socket.accept()
    while True:
        data = client_socket.recv(1024).decode()
        if not data:
            break
        print("수신 데이터: ", data)

        if data == '1':
            obj_tracking_running = True
            # 객체 추적 및 서보 제어 로직 활성화
        elif data == '0':
            obj_tracking_running = False
            # 객체 추적 및 서보 제어 로직 비활성화
        else:
            print("알 수 없는 명령")
    client_socket.close()
```

- 회전자동모드 라즈베리 파이는 TCP 서버로 작동하여 메인 서버로부터 '1' 또는 '0'을 수신하고, 이에 따라 얼굴 인식 및 팬-틸트 메커니즘 제어를 활성화하거나 비활성화한다.

### 2. RaspberryPi-SmartPhone 통신 (승재)
> ### 소스코드 적기
- 소스코드 설명 적기
- 소스코드 설명 적기
  
### 3. RaspberryPi-PC(Web) 통신 (유빈)
- 사용 기술
    - Node.js
    - Express
> ### 소스코드 적기
```javascript
// service_1/node-server/app.js

// express moudle을 로드.
const express = require('express');
const app = express();


// router로 만들어둔 API 로드
const api = require('./api');

app.use(express.json());

// '/api/v1/* 로 api 인터페이스 설정 
app.use('/api/v1', api);

app.use(express.static('public'));

// '/'에서 제작한 hmtl 파일을 제공받을 수 있도록 설정
app.get('/', (req, res) => {
  res.sendFile(__dirname + '/public/index.html');
});

app.listen(3000, () => {
  console.log('server is running on port 3000');
});
```
- 미리 구현한 API를 사용해서, 각 기능들을 수행할 수 있도록 구현하였다.
  
> ### 소스코드 적기
```javascript
// service_1/node-server/module/util.js 63:84

/**
 * run process
 * @param {*} type power, rotation, timer
 * @param {*} mode on, off, auto
 */
function runProcess(type, mode) {
  exec(`sudo ./web_command_exec ${type}${mode}`, (err, stdout, stderr) => {
    if (err) {
      console.log(err);
      return;
    }
    console.log(stdout);
  });
}

module.exports = {
  getData,
  setData,
  runProcess
};
```
- Node.js에서 mq로 전달받은 메시지를 실행시키기 위해, exec를 사용하여 실행시키도록 구현하였다.
  
> ### 소스코드 적기
```javascript
 // service_1/node-server/public/script.js 32:38

fetch('/api/v1/power', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
  },
  body: JSON.stringify({ power }),
});
```
- fetch를 사용하여, API를 호출하도록 구현하였다.

```javascript
// service_1/node-server/api.js 27:36

router.post('/power', (req, res) => {
  const power = req.body.power;
  try{
    runProcess('P', power);
  }
  catch(e){
    res.send({error: e})
  }
  res.send({power})
});

```
- 호출받은 API에서 전달받은 메시지를 파싱하여, exec를 사용하여 실행시키도록 구현하였다.

## 추가 기술 활용 : 얼굴 인식 기술을 활용한 회전 자동 모드의 구현

- 신경망 기반의 탐지기와 팬-틸트 메커니즘을 통해 얼굴을 정확하고 효율적으로 감지하고, 카메라 방향을 동적으로 조절한다. 
- 이 기능은 사용자의 얼굴을 화면 중앙에 유지하며, 실시간 처리 기능을 통해 원활하고 빠른 반응을 제공한다.
- 특히, 멀티 쓰레딩과 소켓 기반 서버를 활용한 원격 제어 기능이 핵심이다.

> ### 신경망 기반 얼굴 탐지

```python
# Caffe 모델을 사용한 신경망 기반 얼굴 탐지
detector = cv2.dnn.readNetFromCaffe("deploy.prototxt.txt", "res10_300x300_ssd_iter_140000.caffemodel")
frame = cv2.resize(frame, (300, 300))
blob = cv2.dnn.blobFromImage(frame, 1.0, (300, 300), (104.0, 177.0, 123.0))
detector.setInput(blob)
detections = detector.forward()
```

- OpenCV의 DNN 모듈을 사용하여 Caffe 프레임워크에서 훈련된 신경망 모델을 로드한다. 이 모델은 카메라에서 캡처된 프레임을 처리하여 얼굴을 감지하는 데 사용된다.

> ### 팬-틸트 메커니즘 제어

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

> ### 멀티 쓰레딩 기술 활용

```python
# 얼굴 탐지 및 서보 제어 쓰레드 시작
processObjectCenter = threading.Thread(target=obj_center, args=(args,))
processObjectCenter.start()

# 팬-틸트 제어 쓰레드 시작
servoControl = threading.Thread(target=set_servos)
servoControl.start()
```

- 멀티 쓰레딩을 이용하여 얼굴 탐지, 팬-틸트 메커니즘 제어, 소켓 서버 통신 등을 동시에 수행한다. 이를 통해 각 기능이 서로 방해받지 않고 효율적으로 동작하며, 시스템의 전체적인 반응 속도와 안정성을 향상시킨다.

> ### 소켓 기반 서버를 통한 원격 제어

```python
# 소켓 서버 초기화 및 설정
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))
server_socket.listen(1)
```

- 서버 소켓을 초기화하여 특정 포트에서 연결 요청을 기다린다. 클라이언트로부터의 명령을 통해 시스템을 제어한다.

> ### 자동 모드 실행과 중지

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



## 📖 실행 방법

> ### Remote controller
라즈베리파이-라즈베리파이 간의 TCP 연결을 위한 컨트롤러 
- 컴파일
```bash
$ gcc -o remote_client remote_client.c -lwiringPi # 리모컨 클라이언트
```

- 실행
```bash
$ sudo ./remote_client # 리모컨 클라이언트 실행
```
> ### Service 1
모든 컨트롤러의 요청을 받는 메인서버
- 컴파일
```bash
$ gcc -o main_server main_server.c -lrt -lwiringPi -lpthread  # 메인 서버
$ gcc -o dht_sensor dht_sensor.c -lrt -lwiringPi - lpthread # 온습도 센서
$ gcc -o ultrasonic_sensor ultrasonic_sensor.c -lrt -lwiringPi -lpthread # 초음파 센서
$ gcc -o bluetooth bluetooth.c -lrt -wiringPi # 블루투스 UART통신 센서
$ gcc -o server_exec server_exec.c -lrt # Web 통신 서버
$ gcc -o TCP_controlelr_connect TCP_controller_connect.c -lrt # Controller와 연결하기 위한 서버
$ gcc -o TCP_rotate_connect TCP_rotate_connect.c -lrt -lwiringPi # Service2 와 연결하기 위한 서버
```

- 실행
```bash
$ sudo ./main_server # 메인서버 실행
$ sudo ./bluetooth # UART 통신 서버 실행
$ sudo ./TCP_controlelr_connect # Controller 와 통신서버 실행
$ sudo ./TCP_rotate_connect # Service2 와 연결하기 위한 서버 실행
```

- node server 실행
```bash
$ cd service_1/node-server/
$ sudo node app.js
```

> ### Service 2
메인서버로 부터 회전자동모드 요청을 받는 회전자동모드(얼굴인식) 서버
- 실행
```bash
$ python3 rotate_auto_server.py # 회전자동모드 서버 실행
```

<br/>

## 💻 조작 방법
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



## 📆 프로젝트 일정
![image](https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/e05f2742-e023-47d1-99c6-19890a1c61de)
## 🍱 최종 결과물
<img src="https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/8cae6907-80c1-4009-863d-0e8d9feb3906" width="500" height="700"/>
<img src="https://github.com/Sonny-Kor/smart-embedded-fan/assets/46300191/ef432f61-e350-4e1f-9c26-0bf249833d67" width="500" height="700"/>

## 🎥 시연 연상
<a href="https://www.youtube.com/watch?v=0BBzzAZkF_s">
    <img src="http://img.youtube.com/vi/0BBzzAZkF_s/0.jpg" width="100%" />
</a>
> 이미지를 클릭하면 유튜브로 이동

## 👨‍👦‍👦 팀 명단
| Profile | Role | Part |
| ------- | ---- | ---- |
| <div align="center"><a href="https://github.com/joon6093"><img src="https://avatars.githubusercontent.com/u/118044367?v=4" width="70px;" alt=""/><br/><sub><b>송제용</b><sub></a></div> | 팀장 | 역할 배분 및 일정 관리, 리모컨 제어 및 상태 표시 기능 개발, 객체 인식 및 추적 기능 개발  |
| <div align="center"><a href="https://github.com/..."><img src="..." width="70px;" alt=""/><br/><sub><b>임유빈</b></sub></a></div> | 팀원 | ... |
| <div align="center"><a href="https://github.com/Sonny-Kor"><img src="..." width="70px;" alt=""/><br/><sub><b>손승재</b></sub></a></div> | 팀원 | ...| 
| <div align="center"><a href="https://github.com/..."><img src="..." width="70px;" alt=""/><br/><sub><b>박성현</b></sub></a></div> | 팀원 | ... | 


## ✍🏻 프로젝트 후기
> ### 송제용
제안 발표 시 교수님의 피드백을 받고 울트라소닉 센서의 한계를 깨닫고, 팬-틸트를 이용한 얼굴 인식으로 사용자 움직임 추적 방향으로 전환했다. 특히, 쓰레딩을 이용해 얼굴 인식과 팬-틸트 메커니즘을 제어하는 과정이 어려웠다. 얼굴 탐지 프레임과 팬-틸트 제어 프레임이 겹쳐서 잘못된 방향으로 움직이는 문제를 시간 제어로 해결했다.

리모컨 라즈베리 파이 개발을 통해 임베디드 개발에 대한 지식이 향상되었고, 얼굴 인식 라즈베리 파이와 리모컨 라즈베리 파이 모두 메인 서버와 TCP 통신을 수행하기 때문에 TCP에 대한 이해도가 높아졌다.

대략 10일이라는 짧은 기간 동안 이 프로젝트를 완성했지만, 구현 내용을 돌이켜보면 어떻게 이렇게 구현했는지 의문스러울 정도로 잘 해낸 것 같다. 10일 프로젝트치고 매우 높은 수준의 구현을 했다고 생각한다. 팀원들 각자가 자신의 역할을 정확히 수행한 덕분에 이렇게 좋은 결과를 낼 수 있었다고 생각한다.

> ### 임유빈

> ### 손승재

> ### 박성
