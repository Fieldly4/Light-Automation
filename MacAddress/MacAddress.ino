    #include <WiFi.h>

    void setup() {
      Serial.begin(115200);
      WiFi.mode(WIFI_STA); // กำหนดโหมดเป็นสถานี
      Serial.print("MAC Address: ");
      Serial.println(WiFi.macAddress());
    }

    void loop() {
      Serial.println(WiFi.macAddress());
      delay(1000);
    }