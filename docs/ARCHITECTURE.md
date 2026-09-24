# ⚡ Waveshare ESP32-S3 Hardware Bridge Architecture
**Dokumenten-Status:** Freigegeben / Architektur-Referenz  
**Zielgruppe:** Hardware-Entwickler, Elektroinstallateure, System-Architekten  
**Hardware-Zielplattform:** Waveshare ESP32-S3 Industrial PoE 8DI / 8RO Controller (RS485 + CAN-Bus)

---

## 1. Executive Summary & Einsatzbereich

Die Waveshare ESP32-S3 Industrial Controller-Familie (speziell die Modelle mit **8 isolierten Digitaleingängen, 8 Relaisausgängen, RS485, CAN-Bus und Ethernet mit PoE**) dient im Sharegy-Ökosystem als universelles **Hutschienen-Edge-Gateway (DIN-Rail Hardware Bridge)**.

Sie schließt die Lücke zwischen klassischer elektrischer Zählerschrank-Hardware (potentialfreie Kontakte, SG-Ready, Modbus RTU, CAN) und der 100 % Cloud-basierten Sharegy Energy OS Plattform.

```
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
```

---

## 2. Hardware-Spezifikation & Pinout-Matrix

### 2.1 Technische Daten

| Baugruppe | Spezifikation | Einsatz bei Sharegy |
| :--- | :--- | :--- |
| **MCU** | ESP32-S3 (Xtensa Dual-Core 32-Bit LX7 bis 240 MHz, 8MB Flash, 512KB SRAM, 2MB/8MB PSRAM) | Lokale Regelschleife, Modbus/CAN-Polling, TLS-Verschlüsselung |
| **Ethernet & PoE** | 10/100M Ethernet (W5500 / LAN8720) + 802.3af PoE (Power over Ethernet) | Primäre Netzwerk- und Stromversorgungsverbindung im Zählerschrank |
| **DC-Versorgung** | 7V bis 36V DC Schraubklemmen (zusätzlich zu PoE) | Redundante Spannungsversorgung über Netzteil (Hutschiene) |
| **Digitale Eingänge (DI)** | 8 Kanäle, Optokoppler-isoliert (5V–36V DC, NPN/PNP schaltbar) | § 14a EnWG Steuerbox-Signale, Rundsteuerempfänger, S0-Zähler |
| **Relaisausgänge (RO/DO)**| 8 Kanäle, Schließer/Öffner (NO/NC), 250V AC / 10A bzw. 30V DC / 10A | SG-Ready Wärmepumpenkontakte, Wallbox-Sperrkontakt, Lastabwurf |
| **RS485-Schnittstelle** | Isoliert mit TVS-Überspannungsschutz & 120Ω Abschlusswiderstand | Modbus RTU Master für Stromzähler (Eastron, Carlo Gavazzi) & WR |
| **CAN-Bus Schnittstelle** | Isoliert mit CAN-Transceiver (ESP32 TWAI Driver) | Batterie-Telemetrie (Pylontech, BYD CAN 2.0B, 500 kBit/s) |
| **Gehäuse** | Robustes Industrie-Hutschienengehäuse (DIN-Rail TS-35) | Direkte Montage auf Verteilerfeld im Zählerschrank |

---

## 3. Signalbelegung & Schaltungsmatrix

### 3.1 Digitale Eingänge: § 14a EnWG Steuerbox-Matrix (DI 1 – DI 4)

Die 8 digitalen Eingänge sind optisch isoliert und werten die potentialfreien Relais der VNB-Steuerbox (oder Rundsteuerempfänger) aus:

| Eingang | VNB-Signal (§ 14a EnWG / BNetzA) | Bedeutung | Firmware-Reaktion |
| :--- | :--- | :--- | :--- |
| **DI 1** | **Kommando 100 % (Normal)** | Keine Netzüberlastung | Volle Ladeleistung für Wallbox, Speicher & Wärmepumpe freigegeben. |
| **DI 2** | **Kommando 60 % (Vorwarnung)** | Leichte Netzengpass-Stufe | Drosselung nicht-kritischer Speicherladungen. |
| **DI 3** | **Kommando 30 % / Dimmung** | Akuter Netzeingriff | Notbremse: Drosselung steuerbarer Großverbraucher auf max. 4,2 kW (bzw. Pmin). |
| **DI 4** | **Kommando 0 % (Sperre / Notaus)** | Kritischer Netzzustand | Vollständige Abschaltung von Wallbox & Wärmepumpen-Verdichter. |
| **DI 5–8** | Frei konfigurierbar | S0-Zählimpulse / PV-Überschusskontakt / Schalter |

### 3.2 Relaisausgänge: SG-Ready & Aktorik (RO 1 – RO 8)

Die Relaisausgänge schalten hardwareseitig über potentialfreie Kontakte:

| Relais | Funktion | Anschlussziel | Typische Beschaltung |
| :--- | :--- | :--- | :--- |
| **RO 1** | **SG-Ready Kontakt 1 (Eingang Klemme 1)** | Wärmepumpe (z.B. Daikin, Viessmann, Vaillant, NIBE) | Ruhezustand: Offen |
| **RO 2** | **SG-Ready Kontakt 2 (Eingang Klemme 2)** | Wärmepumpe | Ruhezustand: Offen |
| **RO 3** | **Wallbox Freigabe / Dimmkontakt** | Wallbox Enable / Key Switch Eingang | Geschlossen = Laden erlaubt, Offen = Gesperrt |
| **RO 4** | **Heizstab Stufe 1 (1.000 W)** | Warmwasser-Heizstab / Schütz | PV-Überschussverwertung |
| **RO 5** | **Heizstab Stufe 2 (2.000 W)** | Warmwasser-Heizstab / Schütz | PV-Überschussverwertung |
| **RO 6** | **Heizstab Stufe 3 (3.000 W)** | Warmwasser-Heizstab / Schütz | PV-Überschussverwertung |
| **RO 7** | **Klimaanlage / Lüftung Freigabe** | HVAC Freigabekontakt | Kühlen bei PV-Spitze |
| **RO 8** | **Alarm / Status-Melderelais** | Externe Warnleuchte / Koppelrelais | Fehlermeldung bei Watchdog-Auslösung |

#### 📋 SG-Ready Zustandsmatrix (RO 1 & RO 2)
* **Betriebszustand 1 (Sperrzeit / 0%):** RO 1 = Geschlossen, RO 2 = Offen (Verdichtersperre durch VNB).
* **Betriebszustand 2 (Normalbetrieb):** RO 1 = Offen, RO 2 = Offen (Standard-Automatik).
* **Betriebszustand 3 (Verstärkter Betrieb / PV-Überschuss):** RO 1 = Offen, RO 2 = Geschlossen (Sollwertanhebung Warmwasser/Heizung).
* **Betriebszustand 4 (Definierte Anlaufempfehlung / Maximallast):** RO 1 = Geschlossen, RO 2 = Geschlossen (Maximaler Speicherbetrieb für PV/Börsenpreis-Tief).

---

## 4. Kommunikationsprotokolle & Datenflüsse

### 4.1 Modbus RTU (RS485)
* **Baudrate:** 9600 oder 19200 Baud, 8 Datenbits, keine Parität (None), 1 Stoppbit (8N1).
* **Unterstützte Geräte:**
  * Eastron SDM630 / SDM72 (Netzbezug, Netzeinspeisung, 3-Phasen Spannungen & Ströme).
  * Sungrow SH5.0–10RT (Hybrid-Wechselrichter Register 5000 ff.).
  * Janitza UMG Serie / Carlo Gavazzi EM24.

### 4.2 CAN-Bus (TWAI Driver)
* **Baudrate:** 500 kBit/s (Standard für Pylontech / BYD CAN 2.0B Protokoll).
* **Extrahierte Telemetriedaten:**
  * Batteriespannung (mV), Strom (mA), Ladestand (SoC in 0,1 %), Temperatur (°C), SOH (%), Alarm-Flags.

### 4.3 MQTT & WebSockets zum Sharegy Backend
* **Topic-Hierarchie:**
  * Telemetrie senden: `sharegy/gateway/{device_id}/telemetry` (alle 1–5 Sekunden).
  * Digital-Input Event: `sharegy/gateway/{device_id}/events/inputs` (sofort bei Signalflanke).
  * Relais-Schaltbefehl empfangen: `sharegy/gateway/{device_id}/commands/relays`
  * Konfiguration & Heartbeat: `sharegy/gateway/{device_id}/status`

---

## 5. Ausfallsicherheit & Fail-Safe Logik (§ 14a Konformität)

Kommt es zu einem Ausfall der Internetverbindung, des WLANs oder der Cloud-Kommunikation, greift die **lokale autonome Sicherheitslogik** der Hardware Bridge:

1. **Cloud-Watchdog (Fallback nach 60 Sekunden):**
   * Bleiben Cloud-Steuerbefehle für mehr als 60 Sekunden aus, wechselt die Firmware automatisch in den autarken lokalen Sicherheitsmodus (*Local Fallback*).
2. **Priorität der Hardware-Eingänge (DI > Cloud):**
   * Ein Hardwaresignal an DI 3 (Dimmung) oder DI 4 (Abschaltung) vom VNB übersteuert **immer** alle Cloud-Befehle mit subsekundärer Reaktionszeit (< 50 ms).
3. **Hardware Watchdog Timer (WDT):**
   * Der interne ESP32-S3 Hardware-Watchdog (Timeout: 5.000 ms) startet das Gerät bei etwaigen Software-Hängern automatisch neu. Relais-Zustände werden im NVS (Non-Volatile Storage) gespeichert.
