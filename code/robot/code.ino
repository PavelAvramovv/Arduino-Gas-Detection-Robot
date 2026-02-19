#define BLYNK_TEMPLATE_ID "TMPL4Rgq396An"
#define BLYNK_TEMPLATE_NAME "Robot v2"
#define BLYNK_AUTH_TOKEN "07ZYPZo7teer4kO2KiuhlS8LLGhcMpCf"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

// Wi-Fi настройки
char ssid[] = "A1_0FD7";  // Име на Wi-Fi мрежата
char pass[] = "75222680";  // Парола за Wi-Fi

// Пинове за моторите
const int motorPin1 = 5;  // GPIO5 (D1)
const int motorPin2 = 4;  // GPIO4 (D2)
const int motorPin3 = 0;  // GPIO0 (D3)
const int motorPin4 = 2;  // GPIO2 (D4)

// Пинове за ултразвуковия сензор
const int trigPin = 12;   // GPIO12 (D6)
const int echoPin = 14;   // GPIO14 (D5)

// Пин за серво мотора
const int servoPin = 15;  // GPIO15 (D8)

// Пин за праховия сензор
const int dustSensorPin = A0; // Аналогов пин за измерване на праховите частици

// Настройки за серво мотора
Servo myServo;
const int servoMin = 0;   // Минимален ъгъл на серво мотора
const int servoMax = 180; // Максимален ъгъл на серво мотора
int currentServoAngle = servoMin; // Текущ ъгъл на серво мотора

// Променливи за ултразвуковия сензор и автопилота
int distance = 0;
int leftDistance = 0;
int rightDistance = 0;
bool autoPilotEnabled = false;
const int obstacleDistanceThreshold = 12; // Праг за разстояние до препятствие в см

// Скорост на моторите (от 0 до 255)
int forwardSpeed = 150;  // Скорост на движение напред
int backwardSpeed = 150; // Скорост на движение назад
int turnSpeed = 150;     // Скорост на завой

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Инициализация на пиновете
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(dustSensorPin, INPUT);

  myServo.attach(servoPin); // Свързване на серво мотора към пина
  myServo.write(90);        // Начална позиция на серво мотора
}

void loop() {
  Blynk.run();
  measureDistance();
  measureDust();

  if (autoPilotEnabled) {
    Serial.println("Autopilot mode active");
    obstacleAvoidance();
  } else {
    Serial.println("Manual control mode");
    // Ръчно управление не трябва да бъде активирано, ако автопилотът е включен
  }
}

// Функция за измерване на разстоянието с ултразвуков сензор
void measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1; // Преобразуване на продължителността в разстояние
  Blynk.virtualWrite(V6, distance); // Изпращане на разстоянието към Blynk
  Serial.print("Distance: ");
  Serial.println(distance);
}

// Функция за измерване на стойността на праховия сензор
void measureDust() {
  int dustValue = analogRead(dustSensorPin);  // Четене на стойността от праховия сензор
  Blynk.virtualWrite(V5, dustValue);  // Изпращане на стойността към Blynk
  Serial.print("Dust Value: ");
  Serial.println(dustValue);
}

// Функция за управление на робота при автопилот
void obstacleAvoidance() {
  if (distance <= obstacleDistanceThreshold) {
    stopMotors();
    backward();
    delay(500); // Увеличете времето, ако е необходимо
    stopMotors();

    // Измерване на разстоянието наляво и надясно
    leftDistance = lookLeft();
    rightDistance = lookRight();
    myServo.write(90); // Връщане на серво мотора в средна позиция

    if (leftDistance > rightDistance) {
      turnLeft();
      delay(500); // Увеличете времето, ако е необходимо
    } else {
      turnRight();
      delay(500); // Увеличете времето, ако е необходимо
    }
    stopMotors();
  } else {
    forward();
  }
}

// Функции за управление на движението на робота с регулиране на скоростта
void forward() {
  analogWrite(motorPin1, forwardSpeed);
  analogWrite(motorPin2, 0);
  analogWrite(motorPin3, forwardSpeed);
  analogWrite(motorPin4, 0);
  Serial.println("Moving forward");
}

void backward() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, backwardSpeed);
  analogWrite(motorPin3, 0);
  analogWrite(motorPin4, backwardSpeed);
  Serial.println("Moving backward");
}

void turnLeft() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, turnSpeed);
  analogWrite(motorPin3, turnSpeed);
  analogWrite(motorPin4, 0);
  Serial.println("Turning left");
}

void turnRight() {
  analogWrite(motorPin1, turnSpeed);
  analogWrite(motorPin2, 0);
  analogWrite(motorPin3, 0);
  analogWrite(motorPin4, turnSpeed);
  Serial.println("Turning right");
}

void stopMotors() {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 0);
  analogWrite(motorPin3, 0);
  analogWrite(motorPin4, 0);
  Serial.println("Motors stopped");
}

// Функции за проверка на разстоянията наляво и надясно
int lookLeft() {
  myServo.write(30);  // Завъртане наляво
  delay(800);
  return measureSingleDistance();
}

int lookRight() {
  myServo.write(150); // Завъртане надясно
  delay(800);
  return measureSingleDistance();
}

int measureSingleDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int cm = (duration / 2) / 29.1;
  return cm;
}

// Функция за активиране и деактивиране на автопилот
BLYNK_WRITE(V8) {  // Auto Pilot Switch
  autoPilotEnabled = param.asInt(); // Включване/изключване на автопилота
  if (autoPilotEnabled) {
    Serial.println("Auto Pilot Enabled");
    forward(); // Започва движение напред при активиране на автопилота
  } else {
    Serial.println("Auto Pilot Disabled");
    stopMotors(); // Спиране на моторите при изключване на автопилота
    myServo.write(90); // Връщане на серво мотора в началната му позиция
  }
}

// Функции за ръчно управление от Blynk
BLYNK_WRITE(V0) {  // Напред
  if (!autoPilotEnabled) {
    int motorState = param.asInt();
    if (motorState == 1) {
      forward();
    } else {
      stopMotors();
    }
  }
}

BLYNK_WRITE(V1) {  // Назад
  if (!autoPilotEnabled) {
    int motorState = param.asInt();
    if (motorState == 1) {
      backward();
    } else {
      stopMotors();
    }
  }
}

BLYNK_WRITE(V2) {  // Ляво завъртане
  if (!autoPilotEnabled) {
    int motorState = param.asInt();
    if (motorState == 1) {
      turnLeft();
    } else {
      stopMotors();
    }
  }
}

BLYNK_WRITE(V3) {  // Дясно завъртане
  if (!autoPilotEnabled) {
    int motorState = param.asInt();
    if (motorState == 1) {
      turnRight();
    } else {
      stopMotors();
    }
  }
}
