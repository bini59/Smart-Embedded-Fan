
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


### 멀티프로세스 - IPC
- 이 프로젝트에서 사용한 멀티프로세스로는 주로 `센서를 읽는 프로세스` 와 `통신을 위한 프로세스`로 이루어져있다. 
- 아래의 코드는 서버와 클라이언트 간의 통신을 위해 생성된 프로세스 예제이다.
> ###  TCP/IP 소켓을 사용한 서버와 클라이언트 간에 데이터 전달 
```c
// service_1/bluetooth.c

// 소켓 생성
if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
}

// 소켓 옵션 설정
if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
}

address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);

// 소켓에 주소 바인딩
if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
}

// 서버 소켓 대기
if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
}
```
- 서버는 TCP/IP 연결을 통해 클라이언트와 연결한 후 대기한다.
> ### POSIX 메시지 큐를 통한 프로세스 간 통신(IPC)
```c
// service_1/bluetooth.c

// 메시지 큐 오픈
mq = mq_open(mq_name, O_WRONLY);
if(mq == (mqd_t)-1) {
    perror("mq_open");
    exit(EXIT_FAILURE);
}
 while(1) {
        printf("Waiting for new connection...\n");
        // 클라이언트 연결 수락
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            continue;
        }
        printf("Connection established\n");
        // 클라이언트와 통신
        while(1) {
            memset(buffer, 0, 1024);
            int valread = read(new_socket, buffer, 1024);
            if(valread == 0) {
                printf("Client disconnected\n");
                break;
            }
            printf("Received: %s\n", buffer);
            
            // 메시지 큐로 데이터 전송
            if(mq_send(mq, buffer, strlen(buffer), 0) == -1) {
                perror("mq_send");
                break;
            }
        }

        close(new_socket);
    }

    // 메시지 큐 닫기
    mq_close(mq);
```
- 클라이언트가 소켓을 통해 데이터를 전송할 때 메시지 큐로 메인프로세스에게 데이터를 전송한다.
### Thread - Mutex

> ### 메인 서버에서 메세지 큐 읽기 스레드 생성
```c
// service_1/main_server.c  421:422

// 메시지 큐 읽기 스레드 생성
pthread_create(&receive_thread, NULL, recv_sensor_data, NULL);
```
- Main서버에서 다른작업(다른 메세지 큐로 부터 메세지를 받는 작업) message queue로 부터 받은 메세지를 읽기 위한 스레드를 생성한다.

> ### 스레드 내에서 메세지 큐 읽기, mutex를 이용한 변수 접근 제한
```c
// service_1/main_server.c  246:265 

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
```
- 생성된 스레드에서 메세지 큐로 부터 메세지를 읽어오고, 메세지를 파싱하여 distance, humidity, temperature 변수에 저장한다. 
- 저장하는 과정에서 다른 스레드에서 변수에 접근하지 못하도록 mutex를 사용하여 잠금을 걸어준다.

### Multi Processing

> ### fork를 이용한 프로세스 분리
```c
// service_1/main_server.c  211:220

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

// service_1/main_server.c  417:419

// 초음파 센서, DHT 센서 프로그램 실행
fork_ultrasonic_sensor();
fork_dht_sensor();

```
- 두 프로그램을 현재 프로세스에서 분리하여 실행시키도록 구현하였다.




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
// service_1/main_server.c 64:84

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
// service_1/TCP_rotate_connect.c 27:58

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
// service_2/rotate_auto_server.py 166:201

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

### 2. RaspberryPi-SmartPhone 통신 
> ### UART통신을 통한 스마트폰 통신 및 IPC 통신
```c
unsigned char serialRead(const int fd) {
    unsigned char x;
    if (read(fd, &x, 1) != 1)
        return -1;
    return x;
}
bool isExistMode(const int fd_serial, const char *isExistmode) {
    char mode = isExistmode[0];
    char value = isExistmode[1] - '0';
    char output[50];

    switch (mode) {
        case 'Q':
            sprintf(output, "종료\n");
            serialWriteString(fd_serial, output);
            return true;
        case 'T':
            if(value == AUTO)
                sprintf(output, "타이머모드 : 자동\n");
            else
                sprintf(output, "타이머모드 : %d분\n", value * 10);
            serialWriteString(fd_serial, output);
            return true;
        case 'R':
            if(value == AUTO)
                sprintf(output, "회전모드 : 자동\n");
            else
                sprintf(output, "회전모드 : %d단\n", value);
            serialWriteString(fd_serial, output);
            return true;
        case 'P':
            if(value == AUTO)
                sprintf(output, "속도모드 : 자동\n");
            else
                sprintf(output, "속도모드 설정 : %d단\n", value);
            serialWriteString(fd_serial, output);
            return true;
        default:
            return false;
    }
}
int main() {
    int fd_serial;
    unsigned char dat, dat2;
    mqd_t mq;
    struct mq_attr attr;
    char message[MAX_MSG_SIZE];

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    if (wiringPiSetup() < 0)
        return 1;

    if ((fd_serial = serialOpen(UART2_DEV, BAUD_RATE)) < 0) {
        printf("Unable to open serial device.\n");
        return 1;
    }

    while (1) {
        if (serialDataAvail(fd_serial)) {
            dat = serialRead(fd_serial);
            if (serialDataAvail(fd_serial)) {
                dat2 = serialRead(fd_serial);
                sprintf(message, "%c%c", dat, dat2);

                if (isExistMode(fd_serial, message)) {
                    if (mq_send(mq, message, 2, 0) == -1) {
                        perror("mq_send");
                    }
                }
            }
        }
        delay(10);
    }

    mq_close(mq);
    return 0;
}
```
- UART통신을 통하여 `serialRead`함수로 2바이트씩 데이터를 읽는다. 
- 받은 데이터를 기반으로 `isExistMode` 함수를 호출하여 모드와 Amount를 확인한다.
- 해당 모드에 따라 응답 메시지를 스마트폰에 전송하고, 메시지 큐에 데이터를 전송한다.
  
### 3. RaspberryPi-PC(Web) 통신
- 사용 기술
    - Node.js
    - Express
> ### express 서버 기본 설정
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
  
> ### child_process를 사용한 C 실행
```javascript
const { exec } = require('child_process');

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
  
> ### 웹 환경에서 API 호출
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
// service_2/rotate_auto_server.py 166:173

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
// service_2/rotate_auto_server.py 174:180

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
// service_2/rotate_auto_server.py 181:184

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
// service_2/rotate_auto_server.py 185:187

# 소켓 서버 초기화 및 설정
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))
server_socket.listen(1)
```

- 서버 소켓을 초기화하여 특정 포트에서 연결 요청을 기다린다. 클라이언트로부터의 명령을 통해 시스템을 제어한다.

> ### 자동 모드 실행과 중지

```python
// service_2/rotate_auto_server.py 188:195

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
<img width="1850" alt="image" src="https://github.com/bini59/Smart-Embedded-Fan/assets/46300191/5013e793-ded0-4d70-8035-9186d639e9df">

## 🍱 최종 결과물

<img src="https://github.com/bini59/Smart-Embedded-Fan/assets/46300191/b5d65aa8-2b0f-441e-9ef1-2cf576802aac" width="500" height="700"/>
<img src="https://github.com/bini59/Smart-Embedded-Fan/assets/46300191/36b4cf98-d1c9-4f2b-a777-fd322ced3894" width="500" height="700"/>

## 🎥 시연 연상
<a href="https://www.youtube.com/watch?v=0BBzzAZkF_s">
    <img src="http://img.youtube.com/vi/0BBzzAZkF_s/0.jpg" width="100%" />
</a>
> 이미지를 클릭하면 유튜브로 이동

## 👨‍👦‍👦 팀 명단
| Profile | Role | Part |
| ------- | ---- | ---- |
| <div align="center"><a href="https://github.com/joon6093"><img src="https://avatars.githubusercontent.com/u/118044367?v=4" width="70px;" alt=""/><br/><sub><b>송제용</b><sub></a></div> | 팀장 | 역할 배분 및 일정 관리, 리모컨 제어 및 상태 표시 기능 개발, 객체 인식 및 추적 기능 개발  |
| <div align="center"><a href="https://github.com/bini59"><img src="https://avatars.githubusercontent.com/u/51144791?v=4" width="70px;" alt=""/><br/><sub><b>임유빈</b></sub></a></div> | 팀원 | Node.js, Express를 이용한 서버 개발. Main Server의 Multi Process, Thread 및 Mutex 구현. |
| <div align="center"><a href="https://github.com/Sonny-Kor"><img src="https://github.com/bini59/Smart-Embedded-Fan/assets/46300191/3929f0b1-1bc3-4444-af1f-9896de0d9497" width="70px;" alt=""/><br/><sub><b>손승재</b></sub></a></div> | 팀원 | 프로세스간 POSIX IPC(메시지 큐)통신 구현, 모터 및 센서 구현 , 블루투스 모듈 통신 구현 | 
| <div align="center"><a href="https://github.com/..."><img src="..." width="70px;" alt=""/><br/><sub><b>박성현</b></sub></a></div> | 팀원 | ... | 


## ✍🏻 프로젝트 후기
> ### 송제용
&nbsp;제안 발표 시 교수님의 피드백을 받고 울트라소닉 센서의 한계를 깨닫고, 팬-틸트를 이용한 얼굴 인식으로 사용자 움직임 추적 방향으로 전환했다. 특히, 쓰레딩을 이용해 얼굴 인식과 팬-틸트 메커니즘을 제어하는 과정이 어려웠다. 얼굴 탐지 프레임과 팬-틸트 제어 프레임이 겹쳐서 잘못된 방향으로 움직이는 문제를 시간 제어로 해결했다.

&nbsp;리모컨 라즈베리 파이 개발을 통해 임베디드 개발에 대한 지식이 향상되었고, 얼굴 인식 라즈베리 파이와 리모컨 라즈베리 파이 모두 메인 서버와 TCP 통신을 수행하기 때문에 TCP에 대한 이해도가 높아졌다.

&nbsp;대략 10일이라는 짧은 기간 동안 이 프로젝트를 완성했지만, 구현 내용을 돌이켜보면 어떻게 이렇게 구현했는지 의문스러울 정도로 잘 해낸 것 같다. 10일 프로젝트치고 매우 높은 수준의 구현을 했다고 생각한다. 팀원들 각자가 자신의 역할을 정확히 수행한 덕분에 이렇게 좋은 결과를 낼 수 있었다고 생각한다.

> ### 임유빈
&nbsp;프로젝트를 해야한다는 것을 전달 받은 후, 선풍기를 개발한다는 아이디어를  들었다. 나는 웹 개발자의 입장으로서, 선풍기의 상태를 확인하고 제어하는 방식이 바로 머리속에 떠올랐다. 

&nbsp;컴퓨터공학과에 입학할 당시, 현실의 물건을 프로그램으로 제어해 자동화 하는것을 꿈꾸며 입학했었다. 그 만큼 더 열정이 생겼다. 또한, 이번 프로젝트를 진행하며, child-process module을 이용해서 자식 프로세스를 새로 생성하는 작업을 진행했다. 평소에는 사용할 일이 없어서 처음 사용해보았다. 사용하면서 기존 진행했던 프로젝트도 위의 모듈을 사용해서 단일 프로그램의 규모를 좀 더 작게 유지할 수 있지 않았을까. 좀 더 생각의 폭을 넓힐 수 있었던 프로젝트 였다고 생각한다. 

&nbsp;또한 많은 부분에서 팀원들의 협력이 있었기에 이 프로젝트가 완성될 수 있었다고 생각한다. 리모컨과, 인공지능, 실제 선풍기의 기동, 하드웨어 연결 등은 다른 팀원들이 맡아서 진행 하였다. 하드웨어는 물론, 코드도 오류 없이 가독성이 좋도록 구현하여 주어서, 코드를 통합하는 과정에서도 큰 어려움이 없이 통합할 수 있었다. 10일간 짧은 기간의 협업이었지만, 기간 내에서 해낼 수 있는 최대한의 결과물이 나왔다고 자신한다.
> ### 손승재
&nbsp; 이번 프로젝트는 팀장님의 창의적인 생각과 나머지 팀원들의 의견이 합쳐져 이번 프로젝트를 시작할 수 있었다. 모두가 각자의 부분을 훌륭하게 구현해주어서 프로젝트의 결과물이 좋았던 것 같다.

&nbsp; 가장 먼저, 모든 팀원들에게 감사의 말을 전하고 싶다. 송제용님은 저에게 평소에 사용할 일이 없었던 POSIX IPC 메시지 큐를 이용한 프로세스 간 통신 구조를 잘 설계할 수 있도록 도와주었다. 서버-클라이언트 구조에 대한 지식이 부족했는데, raspberry, bluetooth, Web 이 3가지의 컨트롤러에서 통신을 할수있도록 설계를 도와주셨다. 송제용 씨 덕분에 많은 것을 배울 수 있었다.

&nbsp; 혼자서 메인서버에서 센서의 값을 쓰레드로 구현하였을 때 제대로 값이 읽히지않는 어려움을 겪었는데, 임유빈님께서 child-process 모듈을 활용해 이 문제를 해결해주었다. 센서의 값을 읽어오는 부분에서 기술적 난관을 극복할 수 있었다. 회로를 설계하고 선풍기를 실제로 동작할 수 있도록 멋지게 만들어주신 박성현님에게도 감사하다.

&nbsp;이번 프로젝트를 통해 협력의 중요성과 의미를 깊이 깨달았다. 처음에 프로젝트를 시작하게 되었을 때, 10일안에 이 프로젝트를 마무리 할 수 있을까? 라는 걱정이 컸었다. 하지만 팀원들 간의 협력 덕분에 빠르게 프로젝트를 마무리할 수 있었고, 결과물도 최고라고 생각하며 많은 기술적인 부분에 대해서 배웠다. 프로젝트에 참여한 팀원들에게 다시 한번 감사의 마음을 전한다.

> ### 박성현
