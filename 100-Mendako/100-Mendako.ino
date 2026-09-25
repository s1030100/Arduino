#include <Servo.h>
#include "Arduino_LED_Matrix.h" // Uno R4 内蔵LEDマトリクス用

ArduinoLEDMatrix matrix;

// ピン定義（ご提示のスケッチに準拠）
const int echoPin = 3;      
const int trigPin = 4;      
const int servoPin = 9;     

Servo headServo;

// 動作制御変数
float currentAngle = 90.0;
float targetAngle = 90.0;
float easing = 0.04;

unsigned long lastSenseTime = 0;
unsigned long lastBlinkTime = 0;
bool isBlinking = false;

// --- 表情のドット絵（8行 x 12列） ---

// 1. 通常の目 (パッチリした目)
byte eyeOpen[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// 2. まばたき (横一本線)
byte eyeBlink[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// 3. にっこり目 (ハの字・笑顔の目)
byte eyeHappy[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void setup() {
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);

  headServo.attach(servoPin);
  headServo.write((int)currentAngle);

  // LEDマトリクス初期化
  matrix.begin();
  matrix.renderBitmap(eyeOpen, 8, 12); // 引数を3つ指定 (配列, 行数8, 列数12)

  randomSeed(analogRead(A0));
}

void loop() {
  unsigned long currentTime = millis();

  // 150msごとに超音波センサーで距離測定
  if (currentTime - lastSenseTime >= 150) {
    lastSenseTime = currentTime;

    float distance = readDistance();

    // 15cm以内に手を近づけたらリアクション！
    if (distance > 0 && distance < 15.0) {
      noticeReaction();
    }
  }

  // 通常時の「まばたき」処理
  if (!isBlinking && (currentTime - lastBlinkTime >= random(3000, 6000))) {
    isBlinking = true;
    lastBlinkTime = currentTime;
    matrix.renderBitmap(eyeBlink, 8, 12); // 一瞬目を閉じる
  }
  if (isBlinking && (currentTime - lastBlinkTime >= 150)) {
    isBlinking = false;
    matrix.renderBitmap(eyeOpen, 8, 12);  // 元の目に戻す
  }

  // サーボの滑らかな首振り動作
  currentAngle += (targetAngle - currentAngle) * easing;
  headServo.write((int)currentAngle);

  delay(20);
}

// 超音波距離計測関数
float readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float distance = pulseIn(echoPin, HIGH, 30000) / 58.00;
  return distance;
}

// 近づいたときのリアクション（にっこり目 ＋ 首傾げ）
void noticeReaction() {
  // ① 目を「にっこり」に変更
  matrix.renderBitmap(eyeHappy, 8, 12);

  // ② 首をクイッと傾ける（120度）
  headServo.write(120);

  // ③ 1.2秒間そのまま手を見つめる
  delay(1200);

  // ④ 元の姿勢・表情に戻す
  headServo.write(90);
  currentAngle = 90.0;
  targetAngle = 90.0;
  matrix.renderBitmap(eyeOpen, 8, 12);

  delay(500); // 連続反応防止のインターバル
}