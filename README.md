# Greenhouse-Management-System
Smart Greenhouse Management System using LPC1768 (ARM Cortex-M3) with temperature, soil moisture, LDR and MQ-135 sensors controlling pump, fan, buzzer and LCD.


# 🌱 LPC1768-Based Smart Greenhouse Management System

## 📌 Project Overview
This project is a **Smart Greenhouse Management System** built using the **LPC1768 (ARM Cortex-M3)** microcontroller.  
It monitors key greenhouse parameters and automatically controls irrigation and ventilation.

### ✅ Sensors Used
- 🌡️ Temperature Sensor (LM35)
- 💧 Soil Moisture Sensor
- 💡 Light Sensor (LDR)
- 🧪 Gas Sensor (MQ-135)

### ✅ Actuators Used
- 💦 Water Pump (Relay Controlled)
- 🌀 12V Brushless DC Fan (Relay Controlled)
- 🔔 Buzzer (Alert)
- 📟 16x2 LCD Display (4-bit mode)

---

## 🎯 Features
✅ Displays sensor values on LCD  
✅ Automatic irrigation based on soil moisture level  
✅ Automatic lighting based on LDR value  
✅ Fan + buzzer ON when gas level or temperature exceeds threshold  
✅ Smoothing logic for stable sensor readings  

---

## 🔧 Hardware Components
- LPC1768 Development Board
- MQ-135 Gas Sensor
- LM35 Temperature Sensor
- Soil Moisture Sensor
- LDR Sensor
- 16x2 LCD Display
- 12V Brushless DC Fan + Relay
- 5V Water Pump + Relay
- BC547 NPN Transistor (Relay Driver)
- 1K Resistor (Transistor Base)
- Buzzer
- Power Supply (5V and 12V)

---

## 🔌 Pin Connections

### ✅ MQ-135 Gas Sensor
| MQ-135 Pin | LPC1768 Pin |
|-----------|-------------|
| VCC | 5V |
| GND | Common Ground |
| AO | P0.24 (ADC) |

### ✅ LM35 Temperature Sensor
| LM35 Pin | LPC1768 Pin |
|----------|-------------|
| VCC | 3.3V |
| GND | Common Ground |
| OUT | P0.25 (ADC) |

### ✅ LDR Sensor
| LDR Connection | LPC1768 Pin |
|---------------|-------------|
| Divider Output | P1.26 (ADC) |
| VCC | 3.3V |
| GND | Common Ground |

### ✅ Soil Moisture Sensor
| Moisture Pin | LPC1768 Pin |
|--------------|-------------|
| VCC | 3.3V |
| GND | Common Ground |
| A0 | P1.23 (ADC) |

### ✅ LCD 16x2 (4-bit)
| LCD Pin | LPC1768 Pin |
|--------|-------------|
| RS | P0.10 |
| EN | P0.11 |
| D4 | P0.19 |
| D5 | P0.20 |
| D6 | P0.21 |
| D7 | P0.22 |

### ✅ 12V Fan Relay (via BC547)
| Connection | Description |
|-----------|-------------|
| Fan Relay Control | P1.27 (via 1K resistor to BC547 base) |
| Relay COM | 12V |
| Relay NO | Fan (+ve) |
| Fan (-ve) | Common Ground |

### ✅ 5V Water Pump Relay
| Connection | Description |
|-----------|-------------|
| Pump Relay Control | P1.28 |
| Relay COM | 5V |
| Relay NC | Pump (+ve) |
| Pump (-ve) | Common Ground |

---

## ⚙️ Working Logic
- If **Moisture % < threshold** → Pump ON ✅
- If **Light % < threshold** → LEDs ON ✅
- If **Gas % > threshold OR Temperature > threshold** → Fan + Buzzer ON ✅  
- LCD displays sensor readings continuously

---

## 🧠 Software & Tools
- **Keil µVision** (Embedded C)
- **Flash Magic** (HEX Uploading)
- **LPC17xx CMSIS Library**

---

## 📂 Project Files
- Source Code: `Code/main.c`
- Pin Connections: `Circuit/Pin_Connections.txt`

---

## 🚀 Future Enhancements
- Add ESP8266/Wi-Fi module for IoT dashboard
- Data logging and graphical monitoring
- Add humidity sensor and greenhouse door automation

---

## 👨‍💻 Author
**Yashas Sai G**  
ECE | Embedded Systems | LPC1768 Projects
