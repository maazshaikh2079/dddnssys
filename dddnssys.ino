// Driver Drowsiness Detection & Escalation/Safety System

const uint8_t MOTOR_EN  = 9;    // L298N ENA (PWM)
const uint8_t MOTOR_IN3 = 12;   // L298N IN3
const uint8_t MOTOR_IN4 = 13;   // L298N IN4

const uint8_t BUZZER_PIN = 5;  // buzzer
const uint8_t GREEN_LED  = 4;
// const uint8_t YELLOW_LED = 3;
const uint8_t RED_LED    = 2;

enum DriverState {AWAKE, SLEEPY};
DriverState state = AWAKE;

unsigned long sleepyStart = 0;
bool redOn = false;
bool motorSlowing = false;

void setup() {
  Serial.begin(9600);

  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  stopMotor();
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

void loop() {
    if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'O') {   // Eyes Open
      setAwake();
    }
    else if (c == 'D') { // Eyes Closed -> Sleepy
      setSleepy();
    }
  }

  // If sleepy, escalate with time
  if (state == SLEEPY) {
    unsigned long elapsed = millis() - sleepyStart;

    // Stage 1: after 5s -> red LED ON
    if (elapsed >= 5000 && !redOn) {
      digitalWrite(RED_LED, HIGH);
      redOn = true;
      digitalWrite(BUZZER_PIN, HIGH);
    }

    // Stage 2: after 10s -> slow motor to stop
    if (elapsed >= 10000 && !motorSlowing) {
      gradualStopMotor();
      motorSlowing = true;
    }
  }
}

void setAwake() {
  state = AWAKE;
  sleepyStart = 0;
  redOn = false;
  motorSlowing = false;

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  motorForward(200);
}

void setSleepy() {
  if (state != SLEEPY) {
    state = SLEEPY;
    sleepyStart = millis();
    redOn = false;
    motorSlowing = false;

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
  }
}

void motorForward(uint8_t speed) {
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(MOTOR_IN3, HIGH);
  digitalWrite(MOTOR_IN4, LOW);
  analogWrite(MOTOR_EN, speed);
}

void stopMotor() {
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  analogWrite(MOTOR_EN, 0);
  digitalWrite(BUZZER_PIN, LOW);
}

void gradualStopMotor() {
  for (int spd = 200; spd >= 0; spd -= 30) {
    motorForward(spd);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
  }
  stopMotor();
}
