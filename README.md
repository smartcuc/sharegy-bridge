# ⚡ Sharegy Hardware Bridge (Waveshare ESP32-S3 DIN-Rail)

[![Build & Release Firmware](https://github.com/smartcuc/sharegy-bridge/actions/workflows/build_firmware.yml/badge.svg)](https://github.com/smartcuc/sharegy-bridge/actions/workflows/build_firmware.yml)
[![License: Proprietary](https://img.shields.io/badge/License-Proprietary-blue.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--S3-orange.svg)](https://platformio.org/)
[![Firmware Version](https://img.shields.io/badge/Firmware-v1.0.0-emerald.svg)](https://github.com/smartcuc/sharegy-bridge/releases)

Offizielle Industrie-Firmware für das universelle **Hutschienen-Edge-Gateway (DIN-Rail Hardware Bridge)** der [Sharegy Energy OS](https://github.com/smartcuc/sharegy) Plattform.

Zielplattform: **Waveshare ESP32-S3 Industrial PoE 8DI / 8RO Controller** (inkl. RS485 Modbus RTU, CAN-Bus TWAI und W5500 Power-over-Ethernet).

---

## 🏛️ System-Architektur

Die Hardware Bridge schließt die Lücke zwischen klassischer Zählerschrank-Hardware (SG-Ready, § 14a Steuerboxen, RS485 Modbus Stromzählern, CAN Batterie-Telemetrie) und dem Cloud Energy OS sowie dem [Moniy Operations & Fleet Hub](https://github.com/smartcuc/moniy):

`
                  ┌──────────────────────────────────────────────────────────┐
                  │                 Sharegy Cloud Energy OS                  │
                  │   (Dynamische Tarife, § 14a Clearing, VPP, Abrechnung)   │
                  └────────────────────────────┬─────────────────────────────┘
                                               │ MQTT / TLS / WebSockets
                                               │ (über Ethernet PoE / WiFi)
                  ┌────────────────────────────▼─────────────────────────────┐
                  │       Waveshare ESP32-S3 Industrial Hardware Bridge      │
                  │   (Dual-Core 240MHz, FreeRTOS, Fail-Safe, Watchdog)      │
                  └───────┬──────────────┬──────────────┬─────────────┬──────┘
                          │              │              │             │
              ┌───────────▼──┐   ┌───────▼──────┐ ┌─────▼─────┐ ┌─────▼─────┐
              │ 8x Relais    │   │ 8x DI Eing.  │ │ RS485     │ │ CAN-Bus   │
              │ (DO 250V/10A)│   │ (Optokoppler)│ │ (Modbus)  │ │ (TWAI)    │
              └───────┬──────┘   └───────┬──────┘ └─────┬─────┘ └─────┬─────┘
                      │                  │              │             │
              ┌───────▼──────┐   ┌───────▼──────┐ ┌─────▼─────┐ ┌─────▼─────┐
              │ • SG-Ready   │   │ • § 14a VNB  │ │ • SDM630  │ │ • Pylon-  │
              │   Klemme 1/2 │   │   Steuerbox  │ │ • Sungrow │ │   tech    │
              │ • Wallbox EN │   │ • Rundsteuer-│ │ • SMA WR  │ │ • BYD HVS │
              │ • Heizstab 1-3   │   empfänger  │ │ • Janitza │ │ • Victron │
              └──────────────┘   └──────────────┘ └───────────┘ └───────────┘
`

---

## ⚙️ Hardware-Spezifikation

| Baugruppe | Spezifikation |
| :--- | :--- |
| **MCU** | ESP32-S3 Xtensa Dual-Core 32-Bit LX7 bis 240 MHz, 8MB Flash, 512KB SRAM |
| **Netzwerk & Stromversorgung** | 10/100M W5500 SPI Ethernet mit IEEE 802.3af PoE + redundanter 7V–36V DC Schraubklemme |
| **8x Digitale Eingänge (DI)** | Optokoppler-isoliert (5V–36V DC) für § 14a EnWG Steuerboxen & Rundsteuerempfänger |
| **8x Relaisausgänge (RO/DO)** | Potentialfreie Schließer/Öffner (NO/NC), 250V AC / 10A für SG-Ready & Lastabwurf |
| **RS485 Schnittstelle** | Isoliert mit TVS-Überspannungsschutz & 120Ω Abschlusswiderstand (Modbus RTU Master) |
| **CAN-Bus Schnittstelle** | Isoliert mit CAN-Transceiver (ESP32 TWAI Driver 500 kBit/s für Pylontech/BYD) |
| **Montage** | Robustes Industrie-Hutschienengehäuse (DIN-Rail TS-35) |

---

## 🔌 Klemmenbelegung & Schaltmatrix

### § 14a EnWG Steuerbox-Eingänge (DI 1 – DI 4)
* **DI 1:** 100 % Normalbetrieb (Keine Netzdrosselung)
* **DI 2:** 60 % Vorwarnung (Leichte Netzüberlastung)
* **DI 3:** 30 % Akute Dimmung (Drosselung steuerbarer Großverbraucher auf max. 4,2 kW)
* **DI 4:** 0 % Abschaltung / Sperre (Kritischer Netzeingriff)

### SG-Ready & Relais-Aktorik (RO 1 – RO 8)
* **RO 1 / RO 2:** SG-Ready Wärmepumpen-Kontakte (Klemme 1 & 2 für Zustände 1–4)
* **RO 3:** Wallbox Freigabe / Dimmkontakt
* **RO 4 – RO 6:** Heizstab Kaskade Stufe 1 (1kW), Stufe 2 (2kW), Stufe 3 (3kW)
* **RO 7:** Klimaanlage / HVAC Freigabe
* **RO 8:** Status- & Alarmmelderelais

---

## 🚀 Flashen & Inbetriebnahme

### 1. Build mit PlatformIO
`ash
# Repository klonen
git clone https://github.com/smartcuc/sharegy-bridge.git
cd sharegy-bridge

# Firmware kompilieren
pio run -e waveshare_esp32s3_poe

# Über USB direkt flashen
pio run -e waveshare_esp32s3_poe -t upload
`

### 2. 1-Click Flashen via esptool (Fertiges Binary)
`ash
python -m esptool --chip esp32s3 --port COM3 --baud 921600 write_flash 0x0 merged_firmware.bin
`

---

## 🛰️ Remote OTA Updates & Moniy Management

Die Bridge verfügt über eine integrierte Remote-OTA-Engine:
* Ausgelöst durch das zentrale **Moniy Fleet Management** via JSON-RPC:
  `json
  {
    "jsonrpc": "2.0",
    "id": 1,
    "method": "rpc.ota_update",
    "params": {
      "url": "https://mon.smartevo.de/firmware/waveshare_esp32s3_bridge_1.1.0.bin",
      "md5": "d41d8cd98f00b204e9800998ecf8427e"
    }
  }
  `
* Die Bridge streamt das neue Image asynchron über einen FreeRTOS-Hintergrund-Task in die Flash-Partition, validiert die Prüfsumme und startet selbstständig neu.

---

## 🔒 Fail-Safe & Watchdog

1. **Cloud-Watchdog (Fallback nach 60s):** Bleiben Cloud-Steuerbefehle aus, schaltet die Bridge automatisch in den autarken Normalbetrieb (*SG-Ready Normal*).
2. **Hard Hardware Priority:** Ein VNB-Signal an DI 3/4 übersteuert **immer** alle Cloud-Befehle subsekundär (< 50ms).
3. **Hardware Watchdog (WDT):** 8.000 ms ESP32-S3 Task-Watchdog mit automatischem Recovery.
