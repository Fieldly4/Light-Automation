// ESP32 ตัวรอง: Slave (รับ ESP-NOW)

#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h> // Library สำหรับ Servo

Servo myServo;
const int SERVO_PIN = 23; // Pin สำหรับ Servo (สาย Signal)
const int LED_PIN = 2;    // Pin สำหรับ LED/Relay

// ⬇️ เพิ่มใหม่: การตั้งค่าโหมด Auto ⬇️
const int LDR_PIN = 34;           // Pin ที่ต่อกับ LDR
const int LIGHT_THRESHOLD = 2800; // ค่าแสงสว่าง
bool isAutoMode = false;          // ตัวแปรเก็บสถานะโหมด Auto (เริ่มต้น = ปิด)
// ⬆️ --------------------------- ⬆️

// องศาสำหรับการ เปิด และ ปิด
#define ANGLE_ON 90
#define ANGLE_OFF 0

// รหัสคำสั่ง (ต้องตรงกับ Sender/Web Server)
#define CMD_ON 100
#define CMD_OFF 101
#define CMD_AUTO_BRIGHTNESS 102 // ⬇️ เพิ่มใหม่ ⬇️: คำสั่งโหมดแสง
// **********************************

// โครงสร้างสำหรับเก็บข้อมูลที่จะรับ
typedef struct struct_message {
    char senderMac[18]; 
    int commandCode;    
} struct_message;

struct_message myData;
 
// ฟังก์ชันที่จะถูกเรียกเมื่อได้รับข้อมูล
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  Serial.println("----------------------------------");
  Serial.print("ได้รับคำสั่ง Code: ");
  Serial.println(myData.commandCode);
  
  // ⬇️ แก้ไข ⬇️: เพิ่มการจัดการ CMD_AUTO_BRIGHTNESS
  switch (myData.commandCode) {
    case CMD_ON:
      // สั่งเปิด (และบังคับปิดโหมด Auto)
      isAutoMode = false; // ⬅️ บังคับปิดโหมด Auto
      Serial.println("-> (บังคับปิดโหมด Auto)");
      
      myServo.write(ANGLE_ON);
      digitalWrite(LED_PIN, HIGH);
      Serial.println("-> ✅ ดำเนินการ: เปิด (Servo + LED)");
      break;
      
    case CMD_OFF:
      // สั่งปิด (และบังคับปิดโหมด Auto)
      isAutoMode = false; // ⬅️ บังคับปิดโหมด Auto
      Serial.println("-> (บังคับปิดโหมด Auto)");
      
      myServo.write(ANGLE_OFF);
      digitalWrite(LED_PIN, LOW);
      Serial.println("-> ✅ ดำเนินการ: ปิด (Servo + LED)");
      break;

    // ⬇️ เพิ่มใหม่ ⬇️: จัดการการสลับโหมด Auto
    case CMD_AUTO_BRIGHTNESS:
      isAutoMode = !isAutoMode; // สลับสถานะ (True/False)
      if (isAutoMode) {
        Serial.println("-> ✅ เปิดใช้งาน โหมดแสงสว่าง (Auto Mode)");
      } else {
        Serial.println("-> ✅ ปิดใช้งาน โหมดแสงสว่าง (Auto Mode)");
      }
      break;
    // ⬆️ --------------------------- ⬆️

    default:
      Serial.println("-> ⚠️ คำสั่งไม่รู้จัก");
      break;
  }
  
  Serial.print("-> สถานะ LED: ");
  Serial.println(digitalRead(LED_PIN) ? "ON" : "OFF");
  Serial.print("-> องศา Servo: ");
  Serial.println(myServo.read());
  Serial.println("----------------------------------");
}
 
void setup() {
  Serial.begin(115200);
  
  // 1. ตั้งค่า Servo
  myServo.attach(SERVO_PIN); 
  myServo.write(ANGLE_OFF);  // เริ่มต้นที่ "ปิด"
  
  // 2. ตั้งค่า LED/Relay
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // เริ่มต้น "ปิด"

  // 3. ⬇️ เพิ่มใหม่ ⬇️: ตั้งค่า LDR
  pinMode(LDR_PIN, INPUT); // ตั้งค่าขา LDR เป็น Input
  isAutoMode = false;      // เริ่มต้นเป็นโหมด Manual
  // ⬆️ -------------------- ⬆️

  // (ส่วนของ ESP-NOW ยังคงเดิม)
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address ของฉัน (Receiver): ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) { Serial.println("❌ Error init ESP-NOW"); return; }
  esp_now_register_recv_cb(OnDataRecv); 
  Serial.println("✔️ ESP-NOW พร้อมรับคำสั่งแล้ว");
}
 
// ⬇️ แก้ไข ⬇️: เพิ่ม Logic การทำงานของโหมด Auto ใน Loop
void loop() {
  // ถ้าไม่ได้อยู่ในโหมด Auto ให้ออกจากฟังก์ชันทันที
  if (!isAutoMode) {
    return;
  }

  // ⬇️ ตรวจสอบแสงทุกๆ 2 วินาที (ป้องกัน Servo/Relay สั่น) ⬇️
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 2000) { // เช็คทุก 2 วินาที
    return;
  }
  lastCheck = millis();
  // ⬆️ ----------------------------------------------- ⬆️

  int ldrValue = analogRead(LDR_PIN); // อ่านค่าแสง (0 = มืดสุด, 4095 = สว่างสุด)
  bool shouldBeOn = (ldrValue > LIGHT_THRESHOLD);
  bool currentState = (digitalRead(LED_PIN) == HIGH); // สถานะปัจจุบัน

  // (สำหรับการ Debug: พิมพ์ค่า LDR)
  Serial.print("โหมด Auto: ค่า LDR = ");
  Serial.print(ldrValue);

  if (shouldBeOn && !currentState) {
    // มันมืด และไฟยังปิดอยู่ -> สั่งเปิด
    Serial.println(" (มืด) -> สั่งเปิด");
    myServo.write(ANGLE_ON);
    digitalWrite(LED_PIN, HIGH);
  } else if (!shouldBeOn && currentState) {
    // มันสว่าง และไฟยังเปิดอยู่ -> สั่งปิด
    Serial.println(" (สว่าง) -> สั่งปิด");
    myServo.write(ANGLE_OFF);
    digitalWrite(LED_PIN, LOW);
  } else {
    // สถานะถูกต้องแล้ว ไม่ต้องทำอะไร
    Serial.println(" (สถานะคงเดิม)");
  }
}