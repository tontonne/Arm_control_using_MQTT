#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Servo.h>
#include <WiFiClientSecure.h>    
#include <MyWifi.h>
// 1. Cấu hình Wi-Fi
// #define WLAN_SSID       "Ton Ton"
// #define WLAN_PASS       "hanoi79981"

// 2. Cấu hình Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883
#define AIO_USERNAME    "TonTon1234"
#define AIO_KEY         "YOUR_ADAFRUIT_IO_KEY"

WiFiClientSecure client; 
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Đăng ký nhận dữ liệu từ các Feed
Adafruit_MQTT_Subscribe servo_sub1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/servo1-angle");
Adafruit_MQTT_Subscribe servo_sub2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/servo2-angle");
Adafruit_MQTT_Subscribe servo_sub3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/servo3-angle");
Adafruit_MQTT_Subscribe servo_sub4 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/servo4-angle");

Servo myServo1;
Servo myServo2;
Servo myServo3;
Servo myServo4;

const int servoPin1 = D1; 
const int servoPin2 = D2; 
const int servoPin3 = D3; 
const int servoPin4 = D4; 

// Các biến quản lý vị trí mượt mà (Bất đồng bộ)
int target1 = 90, current1 = 90;
int target2 = 90, current2 = 90;
int target3 = 90, current3 = 90;
int target4 = 90, current4 = 90;

unsigned long lastServoUpdateTime = 0;
// THAY ĐỔI TỐC ĐỘ TẠI ĐÂY: Khoảng thời gian (ms) giữa mỗi bước dịch chuyển 1 độ.
// Số càng lớn -> Quay càng chậm. (Ví dụ: 15-25ms là vừa đẹp cho cánh tay robot)
const int servoInterval = 20; 

void MQTT_connect();
void updateServosSmoothly();

void setup() {
  Serial.begin(115200);
  
  myServo1.attach(servoPin1,600,2200); myServo1.write(current1);
  myServo2.attach(servoPin2,544,2400); myServo2.write(current2);
  myServo3.attach(servoPin3,544,2400); myServo3.write(current3);
  myServo4.attach(servoPin4); myServo4.write(current4);

  Serial.println();
  Serial.print("Đang kết nối tới ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi đã kết nối!");
  
  client.setInsecure(); //Câu lệnh tối quan trọng đối với kết nối bảo mật trên ESP8266. Nó ra lệnh cho thiết bị bỏ qua việc kiểm tra chứng chỉ dấu vân tay (SSL Certificate) của Adafruit, giúp kết nối luôn thành công mà không sợ bị lỗi khi server cập nhật chứng chỉ mới.

  mqtt.subscribe(&servo_sub1); // Đăng ký chính thức 4 kênh nhận dữ liệu lên lõi MQTT client, ra lệnh cho hệ thống bắt đầu lắng nghe biến động từ các Feed này.
  mqtt.subscribe(&servo_sub2);
  mqtt.subscribe(&servo_sub3);
  mqtt.subscribe(&servo_sub4);
}

void loop() {
  MQTT_connect();
  
  // Gọi hàm cập nhật vị trí servo liên tục trong loop (Không làm nghẽn mạch)
  updateServosSmoothly();

  // Đọc dữ liệu từ Server (Giảm timeout xuống 10ms để nhường khoảng trống cho chuyển động servo)
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10))) {
    if (subscription == &servo_sub1) {
      target1 = constrain(String((char *)servo_sub1.lastread).toInt(), 0, 180);
      Serial.print("Mục tiêu Servo 1: "); Serial.println(target1);
    }
    if (subscription == &servo_sub2) {
      target2 = constrain(String((char *)servo_sub2.lastread).toInt(), 0, 180);
      Serial.print("Mục tiêu Servo 2: "); Serial.println(target2);
    }
    if (subscription == &servo_sub3) {
      target3 = constrain(String((char *)servo_sub3.lastread).toInt(), 0, 180);
      Serial.print("Mục tiêu Servo 3: "); Serial.println(target3);
    }
    if (subscription == &servo_sub4) {
      target4 = constrain(String((char *)servo_sub4.lastread).toInt(), 0, 180);
      Serial.print("Mục tiêu Servo 4: "); Serial.println(target4);
    }
  }
}

// Hàm xử lý di chuyển chậm từng góc một bằng millis()
void updateServosSmoothly() {
  unsigned long currentTime = millis();
  if (currentTime - lastServoUpdateTime >= servoInterval) {
    lastServoUpdateTime = currentTime;

    // Điều khiển mượt Servo 1
    if (current1 < target1) { current1++; myServo1.write(current1); }
    else if (current1 > target1) { current1--; myServo1.write(current1); }

    // Điều khiển mượt Servo 2
    if (current2 < 180 - target2) { current2++; myServo2.write(current2); }
    else if (current2 > 180 - target2) { current2--; myServo2.write(current2); }

    // Điều khiển mượt Servo 3
    if (current3 < target3) { current3++; myServo3.write(current3); }
    else if (current3 > target3) { current3--; myServo3.write(current3); }

    // Điều khiển mượt Servo 4
    if (current4 < target4) { current4++; myServo4.write(current4); }
    else if (current4 > target4) { current4--; myServo4.write(current4); }
  }
}

void MQTT_connect() { // Ý nghĩa: Đầu vòng lặp, luôn gọi hàm kiểm tra trạng thái kết nối MQTT để đảm bảo thiết bị không bị rớt mạng khỏi Server Adafruit IO.
  int8_t ret;
  if (mqtt.connected()) return;

  Serial.print("Đang kết nối MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Thử lại sau 5 giây...");
       mqtt.disconnect();
       delay(5000);
       retries--;
       if (retries == 0) {
         while (1); 
       }
  }
  Serial.println("MQTT đã kết nối!");
}
