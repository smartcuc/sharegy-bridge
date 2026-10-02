<div align="center">

# ⚡ Sharegy Hardware Bridge
### *Industrial DIN-Rail Gateway & § 14a EnWG Controller*

[![Hardware](https://img.shields.io/badge/Hardware-Waveshare_ESP32--S3-E7352C?style=for-the-badge&logo=espressif&logoColor=white)](https://www.waveshare.com/)
[![Firmware](https://img.shields.io/badge/Version-v2.5.0_(Stable)-007ACC?style=for-the-badge&logo=cplusplus&logoColor=white)](version.json)
[![OTA](https://img.shields.io/badge/OTA-1--Klick_Cloud_Update-success?style=for-the-badge&logo=github&logoColor=white)](#-1-klick-cloud-ota-standard)
[![PoE](https://img.shields.io/badge/Ethernet-PoE_W5500_Industrial-blue?style=for-the-badge)](https://www.waveshare.com/)

<p align="center">
  Offizielles Firmware- & Update-Center für die <b>Sharegy DIN-Rail Hardware Bridge</b>.<br/>
  Das universelle Hutschienen-Gateway für <b>§ 14a EnWG Steuerboxen</b>, <b>SG-Ready Wärmepumpen</b>, <b>RS485 Modbus RTU</b>, <b>Wallbox-Dimmung</b> und <b>Smart-Meter-Erfassung</b>.
</p>

---

[📥 Downloads](#-downloads--firmware-binaries) •
[🚀 1-Klick Cloud OTA](#-1-klick-cloud-ota-standard) •
[🔌 Hardware & Specs](#-hardware--schnittstellen) •
[⚙️ Erstinstallation & Flash-Guide](#️-erstinstallation--flash-guide) •
[📝 Changelog](#-versions-historie--changelog)

---

</div>

<br/>

## 📥 Downloads & Firmware Binaries

| Datei | Beschreibung | Typ | Link |
| :--- | :--- | :--- | :--- |
| **`sharegy_bridge_latest.bin`** | Aktuelle OTA-Firmware (v2.5.0) | App-Partition | [📥 Download](./sharegy_bridge_latest.bin) |
| **`merged_firmware.bin`** | Komplettes Factory-Image inkl. Bootloader & Partition-Table | Full Flash (`0x0`) | [📥 Download](./merged_firmware.bin) |
| **`version.json`** | Aktuelle Release-Metadaten für Cloud OTA | JSON Manifest | [📄 Ansehen](./version.json) |

---

## 🚀 1-Klick Cloud OTA (Standard)

Ab Firmware 2.2.0 ist das **1-Klick Cloud-Update** der einfachste und empfohlene Weg:

1. Öffne die Web-UI deiner Bridge im Browser:  
   👉 **`http://sharegy-bridge.local`** *(oder die zugewiesene lokale IP-Adresse)*
2. Die Bridge prüft automatisch die neueste Version auf GitHub.
3. Klicke im Update-Banner auf **`🚀 1-Klick Cloud Update`**.
4. Die Bridge lädt das Binärpaket verschlüsselt herunter, verifiziert die Checksumme, flasht die OTA-Partition und führt einen sauberen Reboot durch.

---

## 💻 Manuelles Web-Upload Update (Fallback)

Falls die Bridge in einem isolierten VLAN ohne direkten Internetzugang betrieben wird:

1. Lade [**`sharegy_bridge_latest.bin`**](./sharegy_bridge_latest.bin) auf deinen PC herunter.
2. Öffne die Web-UI der Bridge (`http://sharegy-bridge.local`).
3. Klappe den Bereich **„📁 Manuelles Datei-Upload (.bin) als Option“** auf.
4. Wähle die `.bin`-Datei aus und klicke auf **„Hochladen & Flashen“**.

---

## 🔌 Hardware & Schnittstellen

Die **Sharegy Hardware Bridge** basiert auf dem industriellen **Waveshare ESP32-S3 PoE Hutschienen-Controller**:

```text
┌─────────────────────────────────────────────────────────────────────────┐
│                     SHAREGY DIN-RAIL HARDWARE BRIDGE                    │
├─────────────────────────────────────────────────────────────────────────┤
│  [ ETH / PoE ]  W5500 10/100M Ethernet (Power over Ethernet IEEE 802.3af)│
│  [ WiFi / BLE]  2.4 GHz 802.11 b/g/n Fallback & Setup-AP                │
│  [ 8x Relais ]  Schließer/Öffner (250V AC / 10A, 30V DC / 10A)           │
│  [ 8x Dig. In]  Optokoppler-Eingänge (5V – 36V DC)                      │
│  [ RS485 Port]  Modbus RTU Master/Slave mit galvanischer Trennung        │
│  [ CAN Port  ]  CAN-Bus 2.0B für Inverter & Batteriespeicher            │
│  [ RTC Clock ]  Echtzeituhr mit Pufferbatterie & NTP-Synchronisation    │
└─────────────────────────────────────────────────────────────────────────┘
```

### Typische Einsatzbereiche:
* **§ 14a EnWG Dimmung & Notabschaltung:** Direkte Anbindung an die Steuerbox des Messstellenbetreibers (MSB) über die digitalen Eingänge (DI1–DI4) zur 4,2 kW Dimmung von Wallboxen und Wärmepumpen.
* **SG-Ready Wärmepumpen-Steuerung:** 4-Stufen-Matrix (Sperre, Normalbetrieb, Anhebung, Zwangseinschaltung) über 2 Relaiskontakte (RO1 & RO2).
* **Modbus RTU Sniffer & Gateway:** Integrierter Protokoll-Sniffer mit Web-Live-Stream (Start/Pause/Clear, Hex-Dump) zur Fehleranalyse an Stromzählern (SDM630, DTSU666) und Wechselrichtern.
* **S0-Impulserfassung:** Hochpräzise Hardware-Interrupts zur Zählung von S0-Strom- und Wasserzählern.

---

## ⚙️ Erstinstallation & Flash-Guide

Wenn eine fabrikneue Hardware geflasht wird (z. B. via USB-C):

### 1. Python & esptool bereitstellen
```bash
pip install esptool
```

### 2. Komplettflash (Factory Image)
Verbinde die Bridge per USB-C mit dem PC:
```bash
# Windows (COM-Port anpassen, z. B. COM3)
esptool.py --chip esp32s3 -p COM3 -b 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 merged_firmware.bin

# Linux / macOS
esptool.py --chip esp32s3 -p /dev/ttyACM0 -b 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 16MB 0x0 merged_firmware.bin
```

---

## 📝 Versions-Historie (Changelog)

### Version 2.5.0 (2026-10-02)
* 🔍 **Modbus Sniffer Suite:** Start, Pause, Clear und Live-Hex-Dump im Web-Dashboard.
* 📈 **Hardware-Ressourcen-Monitor:** Live-Anzeige von RAM-Auslastung (Free Heap), Flash-Speicher, CPU-Temperatur und Uptime.
* ⏱️ **Sofort-NTP-Sync:** Manueller 1-Klick Zeitsynchronisations-Trigger mit Status-Feedback.

### Version 2.3.0 (2026-09-28)
* 🛡️ **Zero-Warning SSL & HTTPS-Kompatibilität:** Vorbereitung für lokale Zertifikate und mTLS.
* 📶 **LAN-First Onboarding:** Zuverlässige Erkennung von PoE-Netzwerkverbindungen vor WLAN-Fallback.

### Version 2.2.0 (2026-09-24)
* 🚀 **1-Klick Cloud OTA:** Vollautomatische Erkennung und Installation neuer Releases via GitHub API und `version.json`.
* 💓 **Moniy Flotten-Integration:** Robuster Application-Level Heartbeat zur Vermeidung von Verbindungsabbrüchen.
* 🏷️ **Standardisierung:** Naming auf `sharegy-bridge` vereinheitlicht.

### Version 1.0.0 (2026-09-20)
* ⚡ **Initial Release:** Unterstützung für 8x Relais, 8x Digital-Inputs, SG-Ready Matrix, § 14a EnWG Dimmung und manuellen Web-Upload.

---

<div align="center">
  <sub>Entwickelt für das <b>Sharegy / smartEvo</b> Energie-Ökosystem.</sub>
</div>
