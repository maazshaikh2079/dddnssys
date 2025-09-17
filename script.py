import cv2
import serial
import time

# === Setup Serial to Arduino ===
arduino = serial.Serial('COM7', 9600, timeout=1)  # change 'COM7' to your port
time.sleep(2)  # allow Arduino to reset

# === Load Haar Cascades ===
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")
eye_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_eye.xml")

cap = cv2.VideoCapture(0)  # webcam index

CLOSE_THRESHOLD = 2.0   # seconds to consider as sleepy
closed_start = None
last_state = None

def send_state(state):
    global last_state
    if state != last_state:   # only send if state changed
        arduino.write(state.encode())
        print("Sent to Arduino:", state)
        last_state = state

while True:
    ret, frame = cap.read()
    if not ret:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, 1.2, 6)

    eyes_open = False
    for (x,y,w,h) in faces[:1]:  # only first face
        roi_gray = gray[y:y+h, x:x+w]
        eyes = eye_cascade.detectMultiScale(roi_gray, 1.1, 3)
        if len(eyes) >= 1:
            eyes_open = True

    now = time.time()
    if eyes_open:
        closed_start = None
        send_state('O')  # Eyes open
    else:
        if closed_start is None:
            closed_start = now
        elapsed = now - closed_start
        if elapsed >= CLOSE_THRESHOLD:
            send_state('D')  # Eyes closed too long
        # else: not yet sleepy -> Arduino keeps last state

    cv2.imshow("Driver Monitor", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
arduino.close()
cv2.destroyAllWindows()
