
# -*- coding: utf-8 -*-
from pyimagesearch.objcenter import ObjCenter
from pyimagesearch.pid import PID
import threading
import pantilthat as pth
import argparse
import signal
import time
import sys
import cv2
import pandas as pd
from imutils.video import VideoStream  # VideoStream 임포트 추가
import socket

# 모터 범위 정의
servoRange = (-90, 90)

# 객체 추적 및 서보 제어 실행 상태 관리
obj_tracking_running = False
# 소켓 서버 실행 상태 관리
socket_server_running = True

# 객체 중심 (x, y) 좌표 및 서보 각도에 대한 전역 변수 정의
objX = 0
objY = 0
move_x = 0
move_y = 0
processObjectCenter = None
servoControl = None

# 키보드 인터럽트 처리 함수
def signal_handler(sig, frame):
    global obj_tracking_running , socket_server_running
    print("[INFO] `ctrl + c`가 눌렸습니다! 종료합니다...")
    obj_tracking_running = False
    socket_server_running = False
    pth.servo_enable(1, False)
    pth.servo_enable(2, False)
    sys.exit(0)  # 프로그램을 강제 종료

def obj_center(args):
    global objX, objY , move_x, move_y
    global obj_tracking_running
    low_confidence_count = 0

    # PiCamera를 이용한 비디오 스트림 초기화 및 준비 시간 대기
    vs = VideoStream(usePiCamera=True).start()
    time.sleep(2.0)
    
    # 무한 루프
    while obj_tracking_running:
        # print("객체 탐지 시작")
        frame = vs.read()
        frame = cv2.flip(frame, 0)
        detector = cv2.dnn.readNetFromCaffe("deploy.prototxt.txt", "res10_300x300_ssd_iter_140000.caffemodel")
        
        frame = cv2.resize(frame, (300, 300))
        blob = cv2.dnn.blobFromImage(frame, 1.0, (300, 300), (104.0, 177.0, 123.0))

        detector.setInput(blob)
        detections = detector.forward()

        # 감지 결과 처리
        column_labels = ["img_id", "is_face", "confidence", "left", "top", "right", "bottom"]
        detections_df = pd.DataFrame(detections[0][0], columns=column_labels)
        detections_df = detections_df[detections_df['is_face'] == 1]

        # 원래 프레임 크기에 맞게 바운딩 박스 좌표 변환
        detections_df['left'] = (detections_df['left'] * 300).astype(int)
        detections_df['bottom'] = (detections_df['bottom'] * 300).astype(int)
        detections_df['right'] = (detections_df['right'] * 300).astype(int)
        detections_df['top'] = (detections_df['top'] * 300).astype(int)

        # 종횡비 계산
        original_size = frame.shape
        target_size = (300, 300)
        aspect_ratio_x = original_size[1] / target_size[1]
        aspect_ratio_y = original_size[0] / target_size[0]

        # 감지된 얼굴별로 반복
        for i, instance in detections_df.head(1).iterrows():
            confidence_score = str(round(100 * instance["confidence"], 2)) + " %"
            if instance["confidence"] < 0.8:
                low_confidence_count += 1
                # 연속으로 5번 낮은 경우
                if low_confidence_count >= 20:
                    objX = 0
                    objY = 0
                    move_x = 0
                    move_y = 0
                    low_confidence_count = 0  # 카운터 초기화
                    print("객체 미탐지로 인한 위치 초기화")
                continue
            
            left = instance["left"]
            right = instance["right"]
            bottom = instance["bottom"]
            top = instance["top"]
            # print("left:",left)
            # print("right:",right)
            # print("bottom:",bottom)
            # print("top:",top)

            # 감지된 얼굴 영역 추출 및 표시
            detected_face = frame[int(top * aspect_ratio_y):int(bottom * aspect_ratio_y), int(left * aspect_ratio_x):int(right * aspect_ratio_x)]
            cv2.putText(frame, confidence_score, (int(left * aspect_ratio_x), int(top * aspect_ratio_y - 10)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
            cv2.rectangle(frame, (int(left * aspect_ratio_x), int(top * aspect_ratio_y)), (int(right * aspect_ratio_x), int(bottom * aspect_ratio_y)), (0, 255, 0), 1)
            
            # 감지된 얼굴의 중심 계산 및 객체 좌표 업데이트 가운데가 130 이므로 -130
            objX = ((left + right) / 2 ) - 130 
            objY = ((top + bottom) / 2 ) - 130
            # print("x:",objX)
            # print("y:",objY)
        
        
        cv2.imshow("Object Center", frame)
        cv2.waitKey(1)

        if not obj_tracking_running:
            break
    

    vs.stop()

def in_range(val, start, end):
    # 입력된 값이 제공된 범위 내에 있는지 확인
    return (val >= start and val <= end)


def set_servos():
    global obj_tracking_running , move_x, move_y
    print("팬틸트 에서 obj_tracking_running:", obj_tracking_running)
    while obj_tracking_running:
        # print("팬틸트 시작")
        # 현재 객체의 위치와 카메라 중심 사이의 차이 계산
        # print("objX:",objX)
        # print("objY:",objY)

        # 팬과 틸트 각도 계산 
        tiltAngle = (objX * 0.2) + move_x
        panAngle = (objY * 0.2) + move_y

        if in_range(tiltAngle, servoRange[0], servoRange[1]):
            pth.tilt(tiltAngle)
            print("tiltAngle:",tiltAngle)
            move_x = tiltAngle

        if in_range(panAngle, servoRange[0], servoRange[1]):
            pth.pan(panAngle)
            print("panAngle:",panAngle)
            move_y = panAngle

        time.sleep(1)

        if not obj_tracking_running:
            break

def socket_server():
    global processObjectCenter, servoControl
    global socket_server_running, obj_tracking_running

    host = '0.0.0.0'  # 모든 인터페이스에서 접속 허용
    port = 7000  # 포트 번호

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(1)
    print("서버가 포트에서 수신 대기 중입니다... port 번호:", port)

    while True:
        client_socket, addr = server_socket.accept()
        print('연결된 주소:', addr)

        while True:
            data = client_socket.recv(1024).decode()
            if not data:
                break
            print("수신 데이터: ", data)

            if data == '1':
                print("1 받음: 객체 탐지 및 서보 제어 시작")
                print("obj_tracking_running :", obj_tracking_running)
                if not obj_tracking_running:
                    obj_tracking_running = True
                    if processObjectCenter is None or not processObjectCenter.is_alive():
                        processObjectCenter = threading.Thread(target=obj_center, args=(args,))
                        processObjectCenter.start()
                    if servoControl is None or not servoControl.is_alive():
                        servoControl = threading.Thread(target=set_servos)
                        servoControl.start()

            elif data == '0':
                print("0 받음: 객체 탐지 및 서보 제어 중지")
                obj_tracking_running = False
                pth.pan(0)
                pth.tilt(0)
                objX = 0
                objY = 0
                move_x = 0
                move_y = 0

            else:
                print("알 수 없는 명령")

        client_socket.close()
        if not socket_server_running:
            break

    server_socket.close()

if __name__ == "__main__":
    # 메인 쓰레드에서 신호 처리기 설정
    signal.signal(signal.SIGINT, signal_handler)

    # 인자 파서 구성 및 인자 파싱
    ap = argparse.ArgumentParser()
    args = vars(ap.parse_args())

    # 서보 활성화
    pth.servo_enable(1, True)
    pth.servo_enable(2, True)
    pth.pan(0)
    pth.tilt(0)

    serverThread = threading.Thread(target=socket_server)
    serverThread.daemon = True 
    serverThread.start()

    while socket_server_running:
        time.sleep(0.1)

    if processObjectCenter is not None and processObjectCenter.is_alive():
        processObjectCenter.join()
    if servoControl is not None and servoControl.is_alive():
        servoControl.join()