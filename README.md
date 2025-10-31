# Light-Automation : ระบบควบคุมแสงสว่างอัจฉริยะ
Physical Computing Project 2025 - IT KMITL

## 💡 ภาพรวมโครงการ (Overview)

ระบบ **Light-Automation** คือโครงการที่มุ่งเน้นการพัฒนาระบบควบคุมการเปิด-ปิดไฟให้เป็นไปโดยอัตโนมัติ (Automation) เพื่อมอบประสบการณ์ **0-Touch** แก่ผู้ใช้งาน

ระบบนี้มีความสามารถในการควบคุมที่หลากหลาย ดังนี้:

1.  **การควบคุมระยะไกล (Remote Control):** ผู้ใช้สามารถสั่งการเปิด-ปิดไฟผ่านแพลตฟอร์มที่กำหนด
2.  **การควบคุมอัตโนมัติตามตัวแปร:** ระบบจะพิจารณาตัวแปรภายนอกเพื่อสั่งการทำงานเองโดยอัตโนมัติ เช่น:
    * **ความสว่างของสภาพแวดล้อม (Ambient Light):** เปิดไฟเมื่อห้องมืดและปิดเมื่อสว่างเพียงพอ
    * **ช่วงเวลา (Time-based Scheduling):** ตั้งเวลาเปิด-ปิดตามเวลาที่กำหนด

---

## 👥 ผู้พัฒนา (Contributors)

| รหัสนักศึกษา | ชื่อ-นามสกุล | รูป |
| :----------: | :----: | :----------: |
| 67070040 | **นายฐนกร โพธิ์ปาน** | <img src="assets/thanakorn_avatar.jpg" width="100" height="100" alt="ฐนกร โพธิ์ปาน"> |
| 67070006 | **นายกษิดิ์เดช แสงสว่าง** | <img src="assets/kasidet_avatar.png" width="100" height="100" alt="กษิดิ์เดช แสงสว่าง"> |

---

## 🛠 แหล่งที่มา (Source)

การควบคุมมอเตอร์ ด้วย Arduino : [https://www.allnewstep.com/article/225/25-arduino-%E0%B8%AA%E0%B8%AD%E0%B8%99%E0%B9%83%E0%B8%8A%E0%B9%89%E0%B8%87%E0%B8%B2%E0%B8%99-arduino-%E0%B8%84%E0%B8%A7%E0%B8%9A%E0%B8%84%E0%B8%B8%E0%B8%A1%E0%B8%A1%E0%B8%AD%E0%B9%80%E0%B8%95%E0%B8%AD%E0%B8%A3%E0%B9%8C-dc-%E0%B8%AB%E0%B8%A1%E0%B8%B8%E0%B8%99%E0%B8%8B%E0%B9%89%E0%B8%B2%E0%B8%A2-%E0%B8%82%E0%B8%A7%E0%B8%B2](https://shorturl.at/oobX7)

การวัดค่าความสว่าง ด้วย Arduino : [https://www.cybertice.com/article/210/%E0%B8%AA%E0%B8%AD%E0%B8%99%E0%B9%83%E0%B8%8A%E0%B9%89%E0%B8%87%E0%B8%B2%E0%B8%99-arduino-%E0%B9%80%E0%B8%8B%E0%B9%87%E0%B8%99%E0%B9%80%E0%B8%8B%E0%B8%AD%E0%B8%A3%E0%B9%8C%E0%B8%A7%E0%B8%B1%E0%B8%94%E0%B8%84%E0%B8%A7%E0%B8%B2%E0%B8%A1%E0%B8%AA%E0%B8%A7%E0%B9%88%E0%B8%B2%E0%B8%87%E0%B8%84%E0%B8%A7%E0%B8%B2%E0%B8%A1%E0%B9%80%E0%B8%82%E0%B9%89%E0%B8%A1%E0%B9%81%E0%B8%AA%E0%B8%87-%E0%B9%80%E0%B8%9B%E0%B8%B4%E0%B8%94%E0%B8%9B%E0%B8%B4%E0%B8%94%E0%B9%84%E0%B8%9F%E0%B8%95%E0%B8%B2%E0%B8%A1%E0%B9%81%E0%B8%AA%E0%B8%87](https://shorturl.at/P0mTf)

ESP32-Arduino_IDE : [https://www.cybertice.com/article/668/%E0%B8%AA%E0%B8%AD%E0%B8%99%E0%B9%83%E0%B8%8A%E0%B9%89%E0%B8%87%E0%B8%B2%E0%B8%99-esp32-%E0%B9%80%E0%B8%A3%E0%B8%B4%E0%B9%88%E0%B8%A1%E0%B8%95%E0%B9%89%E0%B8%99%E0%B9%83%E0%B8%8A%E0%B9%89%E0%B8%87%E0%B8%B2%E0%B8%99-%E0%B8%95%E0%B8%B4%E0%B8%94%E0%B8%95%E0%B8%B1%E0%B9%89%E0%B8%87%E0%B9%82%E0%B8%9B%E0%B8%A3%E0%B9%81%E0%B8%81%E0%B8%A3%E0%B8%A1%E0%B8%A5%E0%B8%87-arduino-ide-2022?gad_source=1&gad_campaignid=23053428197&gbraid=0AAAAADc4pmOuZ5TWqNG6EyQ_qSrwG6y3g&gclid=Cj0KCQjwvJHIBhCgARIsAEQnWlDuYfLiVOycNsoZD6W7MGGgBjBn8W1eSZEYZpCe5_9PJDmVTYteRHgaAvKAEALw_wcB](https://shorturl.at/arHY0)
