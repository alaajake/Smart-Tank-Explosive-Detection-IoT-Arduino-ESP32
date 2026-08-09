# Smart Tank for Explosive / Metal Detection with IoT

### الدبابة الذكية للكشف عن المتفجرات والمعدن باستخدام إنترنت الأشياء

> **Student / Client:** Tabarak Salah Wadah
> **Supervisor:** Mustafa Abdularees

## 📋 Project Overview

IoT-controlled tracked smart tank: Arduino UNO drives DC motors + ultrasonic obstacle avoidance + metal detector sensor, ESP32 hosts WiFi joystick web page with camera stream, auto/manual drive modes, and 360° dance maneuver.

## 🛠️ Technologies Used

- Arduino UNO
- ESP32
- HC-SR04 Ultrasonic
- Metal Detector (VLF) Sensor
- DC Motors + L298N/L293D
- WiFi + Web Server (ESP32)
- SoftwareSerial

## ✨ Key Features

- ✅ AUTO mode: obstacle-avoidance patrol via ultrasonic
- ✅ MANUAL mode: ESP32 web joystick (F/B/L/R + 360° spin)
- ✅ Metal-detection alert: front LED flash + beep + halt + web banner
- ✅ Live webcam/ESP-CAM stream on control page
- ✅ Future upgrade: servo robotic arm for explosive disposal
- ✅ Bidirectional Arduino ↔ ESP32 command protocol (SoftwareSerial 115200)

## 📁 Repository Structure

```
تبارك/
├── src/              # Source code (Python / Arduino .ino)
├── app/              # Desktop / web application
├── models/           # Trained ML models, weights, encodings
├── data/             # Datasets, pickled encodings, databases
├── hardware/         # Arduino sketches (.ino), schematics
├── docs/             # Research papers, Word documents (.docx)
└── assets/           # Images, diagrams, screenshots
```

## 🏗️ Hardware / System Block Diagram

*(Refer to docs folder for detailed schematics, pinouts, and wiring diagrams in the project Word document)*

## 🚀 Setup & Installation

### Python Projects
```bash
# 1. Create virtual environment
python -m venv .venv
.venv\Scripts\activate      # Windows

# 2. Install dependencies
pip install -r requirements.txt   # or installreq.py for automated installs

# 3. Run the application
python app.py
```

### Arduino Projects
1. Install **Arduino IDE** (2.x recommended)
2. Install required libraries via Library Manager (project-specific)
3. Open the `.ino` sketch from `hardware/` folder
4. Select correct board & COM port → Upload

## 🧑‍🎓 Project Context

This project is part of a series of **academic graduation / personal portfolio projects** (Computer Techniques Engineering, 2024-2025). Full research papers, circuit diagrams, and documentation are included in the respective `docs/` directories.
---

## 📝 Copyright & Ownership

**© 2026 Alaa Ahmed Ajeel (علاء أحمد عجيل) - All Rights Reserved**

> **Author & Designer:** Alaa Ahmed Ajeel
> **GitHub:** [@alaajake](https://github.com/alaajake)

This project was fully **developed, written, designed, and implemented** by **Alaa Ahmed Ajeel** as part of academic graduation projects and personal research work.

Customized working copies of these projects have been delivered to clients/students, while this original source code repository remains the property of the author under full copyright protection.

**Unauthorized copying, modification, distribution, or commercial use of this code, via any medium, is strictly prohibited without prior written permission from the author.**
