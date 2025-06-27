# MODBUS RTU SLAVE Firmware Implementation on STM32 MCU

> 🎥 **A video demo is available — see [Live Demo]([#-live-demo](https://www.youtube.com/live/FV6_q8X_ouA?si=fERtnyLyN1bbx7Me))** 

---

##  Table of Contents

- [OVERVIEW](#overview)
- [MODBUS NETWORK EXAMPLE](#modbus-network-example)
- [AVAILABLE STANDARDS](#available-standards)
- [MODBUS MEMORY AREAS](#modbus-memory-areas)
- [CONNECTION DETAILS](#connection-details)
- [RTU FRAME FORMAT](#rtu-frame-format)
- [FUNCTION CODES](#function-codes)
- [live DEMO](#live-demo)
- [THANK YOU](#thank-you)


---

## 🔍 OVERVIEW

- Modbus is an open standard  
- Simple data representation, works in request-response command structure  
- Standard transport, Modbus RTU commands are simple  
- It uses RS485, a differential communication (support 32 nodes in multi-drop configuration)  
- RS485 provides noise immunity better than RS232 electrical standard  
- It requires very little code space, with that developing automation app possible  

![image](https://github.com/user-attachments/assets/229ac572-22b7-47a9-b1c7-010191fb0330)

---



## 📐 AVAILABLE STANDARDS

![image](https://github.com/user-attachments/assets/b809caea-2448-4de2-891c-013f5924e594)


---

## 🧠 MODBUS MEMORY AREAS

| Memory Area        | Type              | Address Range       | Description                     |
|--------------------|-------------------|---------------------|---------------------------------|
| Coils              | Discrete output   | 00001–09999         | Valves, actuators on/off       |
| Inputs             | Discrete input    | 10001–19999         | Switches                       |
| Input Registers    | Analog input      | 30001–39999         | Connect sensor temp, etc.     |
| Holding Registers  | Analog output     | 40001–49999         | Control devices, VFD          |

---

## 🧰 CONNECTION DETAILS

![image](https://github.com/user-attachments/assets/ea8c7c93-9913-49e5-967f-7c9f668f42d8)

---

## 🧾 RTU FRAME FORMAT

**MODBUS SERIAL LINE PROTOCOL DATA UNIT (PDU)**  
```
SLAVE ADDRESS | FUNCTION CODE | DATA | CRC
1 Byte        | 1 Byte        | 0–252 Bytes | 2 Bytes (CRC low | CRC high)
```



## 🔢 FUNCTION CODES

| Function Code | Action          | Table Name                       |
|---------------|------------------|----------------------------------|
| 01 (0x01)     | Read             | Discrete Output Coils            |
| 05 (0x05)     | Write single     | Discrete Output Coils            |
| 15 (0x0F)     | Write multiple   | Discrete Output Coils            |
| 02 (0x02)     | Read             | Discrete Input Contacts          |
| 04 (0x04)     | Read             | Analog Input Register            |
| 03 (0x03)     | Read             | Analog Output Holding Registers  |
| 06 (0x06)     | Write single     | Analog Output Holding Registers  |
| 16 (0x10)     | Write multiple   | Analog Output Holding Registers  |

---

## live DEMO 

> 🎥 **A video demo is available — see [Live Demo]([#-live-demo](https://www.youtube.com/live/FV6_q8X_ouA?si=fERtnyLyN1bbx7Me))**

---


## 🤝 THANK YOU

**Thanks for being a DevHead!**  
Join our community: [https://discord.gg/DevHeads](https://discord.gg/DevHeads)  

