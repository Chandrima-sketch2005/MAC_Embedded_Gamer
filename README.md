# MAC_Embedded_Gamer
Bare-metal ATmega328P multi-sensor gaming platform with a custom C++ pseudo-3D perspective rendering engine on a 128x64 OLED display. Using Arduino Uno, Potentiometer, Flex Sensor, and real-time processing.

# MAC: Multi-Sensor Interactive Gaming Platform & Custom C++ Graphics Pipeline

An embedded system built on the ATmega328P microcontroller that pairs low-latency sensor fusion with a lightweight, fixed-point pseudo-3D perspective rendering engine driving a 128x64 OLED display.

## 📌 Project Overview
The MAC platform combines hardware sensor processing (ultrasonic, film pressure, potentiometer) with a custom graphics pipeline. The system executes a menu-driven Finite State Machine (FSM) written in Embedded C++, incorporating sensor calibration routines, interactive game states, and real-time OLED rendering without using dynamic memory allocation.

## 🤝 Project Type
Group Project

## 🛠️ Hardware & Tools
* **Microcontroller:** ATmega328P (Bare-metal Embedded C++)
* **Display:** 128x64 SSD1306 OLED (4-wire SPI)
* **Sensors:** HC-SR04 Ultrasonic Sensor, Glove-mounted Film Pressure Sensor, Rotary Potentiometer
* **Signal Conditioning:** Voltage divider circuit with $0.1\ \mu\text{F}$ RC low-pass filter
* **Audio & Output:** Passive speaker for tone-matching feedback
* **Toolchain / IDE:** AVR Toolchain / Keil / Arduino IDE

## 🚀 Key Technical Features
* **Custom Pseudo-3D Graphics Engine:** Renders at ~50 FPS within a 1,024-byte SRAM framebuffer using fixed-point perspective projection ($s_x = CX + (wx/Z) \cdot F$), depth-sorted gate rendering, procedural random-walk track generation, and a 3-layer parallax background.
* **Zero Dynamic Memory Allocation:** Optimized memory architecture storing bitmap assets in flash memory (`PROGMEM`) to manage shared 2 KB SRAM efficiently.
* **Hardware Signal Conditioning:** RC filtering and fixed-interval sampling for pressure threshold validation and sensor calibration.
* **Non-Volatile Persistence:** EEPROM integration for high-score tracking across power cycles.

## 📂 Repository Structure
├── src/              # Embedded C++ source code & graphics pipeline
├── include/          # PROGMEM bitmaps, font data, and display drivers
├── hardware/         # Circuit schematics and sensor wiring diagrams
├── media             # Real-time testing videos & images for reference
└── README.md         # Documentation

---
📫 **Contact & Links**
* **Author:** Chandrima Bhattacharyya (Trinity College Dublin)
* **LinkedIn:** [linkedin.com/in/chandrima-bhattacharyya4](https://linkedin.com/in/chandrima-bhattacharyya4)
