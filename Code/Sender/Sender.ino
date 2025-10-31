// ESP32 ตัวหลัก: Web Server (รับคำสั่งจากเว็บ) และ Master (ส่งข้อมูลด้วย ESP-NOW)

#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include "esp_wifi.h"

const char* ssid = ".4G";
const char* password = "";

// MAC Address ของ ESP32 ตัวรับ (Slave)
uint8_t broadcastAddress[] = {};

// รหัสคำสั่ง
#define CMD_ON 100
#define CMD_OFF 101
#define CMD_AUTO_BRIGHTNESS 102
// **********************************

WebServer server(80);

typedef struct struct_message {
    char senderMac[18];
    int commandCode;
} struct_message;

struct_message myData;

// สถานะโหมด Auto
int serverLedState = 0;         // สถานะไฟ (0=OFF, 1=ON)
bool serverAutoModeState = false; // สถานะโหมด Auto (เริ่มต้น = ปิด)


//HTML
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="th">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>💡 ระบบควบคุมไฟ (ESP32)</title>
    <style>
        /* (CSS ทั้งหมดเหมือนเดิม - ไม่มีการเปลี่ยนแปลง) */
        body {
            font-family: Arial, sans-serif; text-align: center; padding: 20px;
            background-color: #333; color: #f0f0f0;
            transition: background-color 1.5s ease-in-out, color 1.5s ease-in-out; 
        }
        body.light-on { background-color: #f4f4f9; color: #333; }
        .container {
            max-width: 500px; margin: 0 auto; background-color: rgba(255, 255, 255, 0.1); 
            padding: 30px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            transition: background-color 1.5s ease-in-out, box-shadow 1.5s ease-in-out;
        }
        body.light-on .container { background-color: #fff; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }
        h1 { color: inherit; }
        .status {
            font-size: 2em; margin: 20px 0; padding: 10px; border-radius: 5px;
            font-weight: bold; color: inherit; background-color: rgba(255, 255, 255, 0.1); 
            border: 1px solid rgba(255, 255, 255, 0.2);
            transition: background-color 1.5s ease-in-out, border 1.5s ease-in-out, color 1.5s ease-in-out;
        }
        .status.off {
            color: #dc3545; background-color: rgba(220, 53, 69, 0.1);
            border: 1px solid rgba(220, 53, 69, 0.3);
        }
        body.light-on .status.on {
            color: #28a745; background-color: #d4edda; border: 1px solid #c3e6cb;
        }
        .timer-info {
            margin-top: 20px; padding: 15px; background-color: rgba(255, 255, 255, 0.1); 
            border-radius: 5px; text-align: left;
            transition: background-color 1.5s ease-in-out;
        }
        body.light-on .timer-info { background-color: #e9ecef; }
        .timer-info p, .timer-info span, .timer-info h2, .timer-info h4, .timer-info label {
            color: #ffc107 !important; font-weight: bold;
            transition: color 1.5s ease-in-out;
        }
        body.light-on .timer-info p, body.light-on .timer-info span, body.light-on .timer-info h2, body.light-on .timer-info h4, body.light-on .timer-info label {
            color: #333 !important; 
        }
        button {
            padding: 10px 20px; margin: 10px; font-size: 1em; cursor: pointer;
            border: none; border-radius: 5px; transition: background-color 0.3s;
            color: white; font-weight: bold;
        }
        #onButton { background-color: #28a745; }
        #onButton:hover { background-color: #218838; }
        #offButton { background-color: #dc3545; }
        #offButton:hover { background-color: #c82333; }
        #resetButton { background-color: rgb(81, 81, 232); }
        #resetButton:hover { background-color: blue; }
        input[type="number"] {
            width: 100px; padding: 5px; font-size: 0.9em;
            border-radius: 5px; border: 1px solid #ccc;
        }
        select {
             padding: 5px; font-size: 0.9em;
             border-radius: 5px; border: 1px solid #ccc;
        }
        #startAutoTimerButton {
            background-color: #007bff; color: white;
            padding: 6px 10px; font-size: 0.9em; margin-left: 10px;
        }
        #startAutoTimerButton:hover { background-color: #0056b3; }
        #cancelAutoTimerButton {
            background-color: #6c757d; color: white; padding: 6px 10px; font-size: 0.9em;
            margin-left: 5px; 
        }
        #cancelAutoTimerButton:hover { background-color: #5a6268; }
    </style>
</head>
<body>

    <div class="container">
        <h1>Light Switch Controller 💡</h1>

        <div id="lightStatus" class="status off">
            Light <strong>Off</strong>
        </div>
        
        <div>
            <button id="onButton" onclick="setLightStatus(true)">On</button>
            <button id="offButton" onclick="setLightStatus(false)">Off</button>
        </div>

        <div style="margin-top: 15px; border-top: 1px solid rgba(128,128,128,0.3); padding-top: 10px;">
            <h4>ฟังก์ชันพิเศษ</h4>
            <p style="margin-bottom: 5px;">สถานะโหมดแสงสว่าง: 
                <strong id="autoStatusDisplay" style="color: #dc3545;">ปิด</strong>
            </p>
            <button id="autoButton" onclick="sendFunctionCommand(102)" style="background-color: #ffc107; color: #333; padding: 10px 20px; margin: 5px;">
                สลับโหมดแสงสว่าง (CMD 102)
            </button>
        </div>
        <div class="timer-info">
            <h2>⏱️ Timer </h2>
            <p>Light <strong>On</strong>: <span id="onTimeDisplay">00:00:00</span></p>
            <p>Light <strong>Off</strong>: <span id="offTimeDisplay">00:00:00</span></p>
            <button id="resetButton" onclick="resetTimers()">reset</button>
            <div style="margin-top: 20px; border-top: 1px solid rgba(128,128,128,0.3); padding-top: 15px;">
                <h4>Auto Timer (ตั้งเวลาอัตโนมัติ)</h4>
                <div style="margin-bottom: 10px;">
                    <label for="autoTimerValue">ตั้งเวลา: </label>
                    <input type="number" id="autoTimerValue" placeholder="ป้อนเวลา" min="1">
                    <select id="autoTimerUnit" style="margin-left: 5px;">
                        <option value="seconds">วินาที</option>
                        <option value="minutes" selected>นาที</option>
                        <option value="hours">ชั่วโมง</option>
                    </select>
                </div>
                <div style="margin-bottom: 10px;">
                     <label for="autoTimerAction">เมื่อครบกำหนด: </label>
                    <select id="autoTimerAction">
                        <option value="off">สั่งปิด (Off)</option>
                        <option value="on">สั่งเปิด (On)</option>
                    </select>
                    <button id="startAutoTimerButton" onclick="startAutoTimer()">Start</button>
                    <button id="cancelAutoTimerButton" onclick="cancelAutoTimer()">Cancel</button>
                </div>
                <p style="margin-top: 10px;">
                    Time remaining (<span id="timerActionDisplay" style="font-weight: bold; color: inherit;">...</span>): 
                    <span id="autoTimerDisplay" style="font-weight: bold; color: inherit;">Not set</span>
                </p>
            </div>
        </div>
    </div>

    <script>
        let isLightOn = %STATE%; 
        let isAutoOn = %AUTO_STATE%;

        let onTimeSeconds = 0;
        let offTimeSeconds = 0;
        let timerInterval;

        let autoTimerIntervalId = null;
        let autoTimerRemainingSeconds = 0;
        let autoTimerActionTarget = null; 

        // ดึงองค์ประกอบ HTML
        const statusDisplay = document.getElementById('lightStatus');
        const onTimeDisplay = document.getElementById('onTimeDisplay');
        const offTimeDisplay = document.getElementById('offTimeDisplay');
        const body = document.body;
        
        const autoTimerDisplay = document.getElementById('autoTimerDisplay');
        const autoTimerValueInput = document.getElementById('autoTimerValue');
        const autoTimerUnitInput = document.getElementById('autoTimerUnit');
        const autoTimerActionInput = document.getElementById('autoTimerAction');
        const timerActionDisplay = document.getElementById('timerActionDisplay');
        const autoStatusDisplay = document.getElementById('autoStatusDisplay');


        // ฟังก์ชันแปลงวินาทีเป็นรูปแบบ HH:MM:SS
        function formatTime(totalSeconds) {
            const hours = Math.floor(totalSeconds / 3600);
            const minutes = Math.floor((totalSeconds % 3600) / 60);
            const seconds = totalSeconds % 60;
            const format = (val) => String(val).padStart(2, '0');
            return `${format(hours)}:${format(minutes)}:${format(seconds)}`;
        }

        // ฟังก์ชันอัปเดตตัวนับเวลาทุกวินาที
        function updateTimers() {
            if (isLightOn) {
                onTimeSeconds++;
            } else {
                offTimeSeconds++;
            }
            onTimeDisplay.textContent = formatTime(onTimeSeconds);
            offTimeDisplay.textContent = formatTime(offTimeSeconds);
        }

        // อัปเดตทั้งสถานะไฟ และสถานะโหมด Auto
        function updateUI() {
            // 1. อัปเดตสถานะไฟ (Light Status)
            if (isLightOn) {
                body.classList.add('light-on');
                statusDisplay.innerHTML = 'Light <strong>On</strong>';
                statusDisplay.className = 'status on';
            } else {
                body.classList.remove('light-on');
                statusDisplay.innerHTML = 'Light <strong>Off</strong>';
                statusDisplay.className = 'status off';
            }
            
            // 2. อัปเดตสถานะโหมด Auto (Auto Mode Status)
            if (isAutoOn) {
                autoStatusDisplay.textContent = 'เปิด';
                autoStatusDisplay.style.color = '#28a745'; // Green
            } else {
                autoStatusDisplay.textContent = 'ปิด';
                autoStatusDisplay.style.color = '#dc3545'; // Red
            }
        }

        function fetchStatus() {
            fetch('/status')
                .then(response => {
                    if (!response.ok) { throw new Error('Network response was not ok'); }
                    return response.json();
                })
                .then(data => {
                    // อัปเดตตัวแปร JavaScript ตามสถานะจริงจาก Server
                    isLightOn = data.light;
                    isAutoOn = data.auto;
                    updateUI(); // วาด UI ใหม่ตามสถานะที่ได้รับ
                })
                .catch(error => console.error('Error fetching status:', error));
        }


        // Auto Timer
        function cancelAutoTimer() {
            if (autoTimerIntervalId) {
                clearInterval(autoTimerIntervalId);
                autoTimerIntervalId = null;
            }
            autoTimerRemainingSeconds = 0;
            autoTimerActionTarget = null;
            autoTimerDisplay.textContent = 'Not set';
            timerActionDisplay.textContent = '...';
            autoTimerValueInput.value = '';
        }
        function updateAutoTimer() {
            if (autoTimerRemainingSeconds > 0) {
                autoTimerRemainingSeconds--;
                autoTimerDisplay.textContent = formatTime(autoTimerRemainingSeconds);
            } else {
                const actionText = autoTimerActionTarget === true ? 'เปิด' : 'ปิด';
                setLightStatus(autoTimerActionTarget, false)
                    .then(() => {
                        alert('ถึงเวลาสั่ง' + actionText + 'ไฟอัตโนมัติแล้ว!'); 
                    })
                    .catch((error) => {
                        console.error('Auto-timer fetch failed:', error);
                        alert('ตั้งเวลาอัตโนมัติล้มเหลว: ไม่สามารถส่งคำสั่งได้!');
                    })
                    .finally(() => {
                        cancelAutoTimer();
                    });
            }
        }
        function startAutoTimer() {
            const value = parseInt(autoTimerValueInput.value);
            const unit = autoTimerUnitInput.value;
            const action = autoTimerActionInput.value; 

            if (isNaN(value) || value <= 0) {
                alert('กรุณาใส่เวลาที่ถูกต้อง (มากกว่า 0)');
                return;
            }

            let totalSeconds = 0;
            if (unit === 'seconds') {
                totalSeconds = value;
            } else if (unit === 'hours') {
                totalSeconds = value * 3600;
            } else {
                totalSeconds = value * 60;
            }
            
            autoTimerActionTarget = (action === 'on');
            if (autoTimerIntervalId) {
                clearInterval(autoTimerIntervalId);
            }
            autoTimerRemainingSeconds = totalSeconds;
            autoTimerDisplay.textContent = formatTime(autoTimerRemainingSeconds);
            timerActionDisplay.textContent = autoTimerActionTarget ? 'สั่งเปิด' : 'สั่งปิด';
            autoTimerIntervalId = setInterval(updateAutoTimer, 1000);
            autoTimerValueInput.value = '';
            alert('ตั้งเวลา ' + (autoTimerActionTarget ? 'เปิด' : 'ปิด') + ' ในอีก ' + formatTime(totalSeconds) + ' สำเร็จ');
        }


        // ฟังก์ชันส่งคำสั่งพิเศษ (Auto)
        function sendFunctionCommand(cmdCode) {
            console.log('Sending special command: ' + cmdCode);

            // อัปเดต UI ทันที (Optimistic Update)
            if (cmdCode === 102) {
                isAutoOn = !isAutoOn; // สลับสถานะที่แสดงผลทันที
                updateUI();
            }

            // ส่งคำสั่งไปหา ESP32 Server
            fetch('/set?cmd=' + cmdCode)
                .then(response => {
                    if (!response.ok) {
                        console.error('Failed to send command');
                        alert('Error: Could not send function command to server!');
                        // ⬇️ ถ้าล้มเหลว ให้ดึงสถานะจริงกลับมา ⬇️
                        fetchStatus(); 
                    } else {
                        console.log('Function command sent: ' + cmdCode);
                        // ไม่ต้อง alert, ให้ poller จัดการอัปเดต UI
                    }
                })
                .catch(error => {
                    console.error('Fetch error:', error);
                    alert('Network error: ' + error.message);
                    fetchStatus(); // ⬇️ ถ้าล้มเหลว ให้ดึงสถานะจริงกลับมา ⬇️
                });
        }


        // (On/Off) ให้ปิดโหมด Auto ด้วย
        function setLightStatus(turnOn, isManualAction = true) {
            // (ป้องกันการกดซ้ำ)
            if (isLightOn === turnOn && isManualAction) {
                return Promise.resolve();
            }

            if (isManualAction) {
                cancelAutoTimer();
            }
            
            // 1. อัปเดตตัวแปร JS และ UI ทันที
            isLightOn = turnOn;
            isAutoOn = false; // ⬅️⬅️ นี่คือฟังก์ชันที่ 2 (ปิดโหมด Auto อัตโนมัติ)
            updateUI(); // ⬅️ เรียกใช้ฟังก์ชันอัปเดต UI ใหม่

            // 2. ส่งคำสั่งไปหา ESP32 Server
            let cmd = isLightOn ? 100 : 101;
            return fetch('/set?cmd=' + cmd) 
                .then(response => {
                    if (!response.ok) {
                        console.error('Failed to send command');
                        throw new Error('Server responded with ' + response.status);
                    }
                    console.log('Command sent: ' + (isLightOn ? 'ON' : 'OFF'));
                })
                .catch(error => {
                    console.error('Fetch error:', error);
                    fetchStatus(); // ⬇️ ถ้าล้มเหลว ให้ดึงสถานะจริงกลับมา ⬇️
                    throw error; 
                });
        }

        // ฟังก์ชันรีเซ็ตตัวนับเวลา
        function resetTimers() {
            onTimeSeconds = 0;
            offTimeSeconds = 0;
            onTimeDisplay.textContent = formatTime(onTimeSeconds);
            offTimeDisplay.textContent = formatTime(offTimeSeconds);
            
            cancelAutoTimer(); 
            alert('รีเซ็ตตัวนับเวลาแล้ว!');
        }
        
        // เริ่มการทำงาน
        // 1. เริ่มตัวจับเวลา (นับเวลา On/Off)
        timerInterval = setInterval(updateTimers, 1000);

        // 2. เริ่มตัว Polling (ถามสถานะ /status)
        setInterval(fetchStatus, 3000); // ถามสถานะทุก 3 วินาที

        // 3. ตั้งค่า UI เริ่มต้น (ตามสถานะที่ Server ส่งมาตอนโหลด)
        updateUI(); // ⬅️ เรียกใช้ฟังก์ชันอัปเดต UI ใหม่
        // ⬆️ -------------------------------- ⬆️
        
    </script>

</body>
</html>
)=====";


// ฟังก์ชันที่จะถูกเรียกเมื่อส่งข้อมูลสำเร็จ/ล้มเหลว
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
    const uint8_t *target_mac = broadcastAddress;
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5]);
    Serial.print("สถานะการส่ง (ถึง MAC เป้าหมาย: ");
    Serial.print(macStr);
    Serial.print("): ");
    Serial.println((status == ESP_NOW_SEND_SUCCESS) ? "✅ สำเร็จ" : "❌ ล้มเหลว");
}

// ⬇️ แก้ไข ⬇️: ฟังก์ชันสร้างหน้าเว็บไซต์หลัก (เพิ่ม AUTO_STATE)
void handleRoot() {
    String stateStr = (serverLedState == 1) ? "true" : "false";
    String autoStateStr = (serverAutoModeState == true) ? "true" : "false"; // ⬅️ เพิ่ม
    
    String html = MAIN_page;
    html.replace("%STATE%", stateStr);
    html.replace("%AUTO_STATE%", autoStateStr); // ⬅️ เพิ่ม
    
    server.send(200, "text/html", html);
}

// ฟังก์ชันสำหรับส่งสถานะให้ JavaScript (Polling)
void handleStatus() {
    // สร้าง JSON response
    // เช่น {"light":true, "auto":false}
    String json = "{";
    json += "\"light\":";
    json += (serverLedState == 1) ? "true" : "false";
    json += ",";
    json += "\"auto\":";
    json += (serverAutoModeState == true) ? "true" : "false";
    json += "}";
    server.send(200, "application/json", json);
}
// ⬆️ -------------------------------- ⬆️


// ฟังก์ชันสำหรับรับคำสั่ง (On/Off/Auto) จาก fetch()
void handleSet() {
    if (!server.hasArg("cmd")) {
        server.send(400, "text/plain", "Bad Request: Missing cmd");
        return;
    }

    int requestedCommand = server.arg("cmd").toInt();
    
    // อัปเดตตัวแปรสถานะบน Server ตามคำสั่ง
    if (requestedCommand == CMD_ON) {
        serverLedState = 1;
        serverAutoModeState = false; // ⬅️ บังคับปิดโหมด Auto
    } else if (requestedCommand == CMD_OFF) {
        serverLedState = 0;
        serverAutoModeState = false; // ⬅️ บังคับปิดโหมด Auto
    } else if (requestedCommand == CMD_AUTO_BRIGHTNESS) {
        serverAutoModeState = !serverAutoModeState; // ⬅️ สลับสถานะโหมด Auto
        // (ไม่ต้องเปลี่ยน serverLedState)
    } else {
        server.send(400, "text/plain", "Unknown Command");
        return;
    }

    // 1. เตรียมข้อมูลสำหรับส่ง
    myData.commandCode = requestedCommand;
    
    // 2. ส่งคำสั่งผ่าน ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    Serial.print("ส่งคำสั่ง Code: ");
    Serial.println(requestedCommand);
    if (result != ESP_OK) {
        Serial.println("⚠️ Error ในการเรียกใช้ฟังก์ชันส่ง ESP-NOW");
    }

    // ตอบกลับ JavaScript ว่ารับทราบ
    server.send(200, "text/plain", "OK");
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(1000); Serial.print("."); }
    Serial.println("\nเชื่อมต่อสำเร็จ!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    WiFi.macAddress().toCharArray(myData.senderMac, 18);
    if (esp_now_init() != ESP_OK) { Serial.println("❌ Error init ESP-NOW"); return; }
    esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK){ Serial.println("❌ ไม่สามารถเพิ่ม Peer ได้"); return; }
    Serial.println("✔️ เพิ่ม Peer (ตัวรับ) สำเร็จ");

    // ส่งคำสั่ง OFF (101) เมื่อเริ่มต้นระบบ
    Serial.println("กำลังส่งคำสั่ง OFF เริ่มต้น (CMD 101)...");
    myData.commandCode = CMD_OFF; // 101
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result != ESP_OK) {
        Serial.println("⚠️ Error ในการส่งคำสั่ง OFF เริ่มต้น");
    }
    serverLedState = 0;      // ⬅️ ตั้งค่าเริ่มต้น
    serverAutoModeState = false; // ⬅️ ตั้งค่าเริ่มต้น

    // เพิ่ม Endpoint สำหรับ /set และ /status
    server.on("/", HTTP_GET, handleRoot);     // ส่งหน้าเว็บหลัก
    server.on("/set", HTTP_GET, handleSet);   // รับคำสั่ง (On/Off/Auto)
    server.on("/status", HTTP_GET, handleStatus); // ⬅️ เพิ่มใหม่: ส่งสถานะให้ JS
    
    server.begin();
    Serial.println("HTTP server เริ่มทำงานแล้ว");
}

void loop() {
    server.handleClient();
}
