# ⚡ Sharegy Hardware Bridge – Firmware & Update-Center

Offizielles Firmware-Repository für die **Sharegy DIN-Rail Hardware Bridge** (Hutschienen-Gateway für SG-Ready, § 14a EnWG Steuerboxen, RS485 Modbus und CAN-Bus).

* **Ziel-Hardware:** Waveshare ESP32-S3 Industrial PoE 8DI / 8RO Controller
* **Aktuelle Version:** 1.0.0 (Stable)
* **Download:** 👉 [**sharegy_bridge_latest.bin**](./sharegy_bridge_latest.bin) *(Rechtsklick -> Ziel speichern unter...)*

---

## 🚀 Firmware aktualisieren (Schritt-für-Schritt)

Du kannst das Update ganz einfach über den Webbrowser auf deinem Smartphone, Tablet oder PC durchführen:

### Variante 1: Über die lokale Weboberfläche (Empfohlen)

1. **Firmware herunterladen:**  
   Lade die Datei [sharegy_bridge_latest.bin](./sharegy_bridge_latest.bin) auf deinen Computer oder dein Smartphone herunter.

2. **Gerät im Browser aufrufen:**  
   * Wenn das Gerät im Heimnetzwerk/WLAN ist: Öffne im Browser **http://sharegy-bridge.local** (oder die lokale IP-Adresse der Bridge).
   * Wenn das Gerät im Einrichtungsmodus ist: Verbinde dich mit dem WLAN Sharegy-WS-ESP32S3-... (Passwort: sharegy!26B) und öffne **http://192.168.4.1**.

3. **Update-Menü öffnen:**  
   * Scrolle nach unten zum Bereich **"Firmware Update (OTA)"** oder klicke im Menü auf **"Update"**.

4. **Datei auswählen & Starten:**  
   * Wähle die heruntergeladene Datei sharegy_bridge_latest.bin aus.
   * Klicke auf **"Update starten"**.

5. **Fertig:**  
   * Der Ladebalken läuft durch (ca. 5–10 Sekunden).
   * Die Bridge startet automatisch neu und meldet sich mit der neuen Firmware-Version an!

---

### Variante 2: Automatisches Cloud-Update via Moniy

Wenn deine Bridge mit dem Internet verbunden ist, werden neue Firmware-Versionen in der Regel **automatisch und unterbrechungsfrei** über das zentrale smartEvo/Moniy Flotten-Management eingespielt. Du musst in diesem Fall nichts manuell tun.

---

### Variante 3: Erstinstallation über USB (Für neue oder zurückgesetzte Geräte)

Falls ein fabrikneues Waveshare-Gerät erstmalig geflasht werden muss:

1. Verbinde die Bridge über ein USB-C Datenkabel mit deinem PC.
2. Führe folgenden Befehl im Terminal aus (Python + esptool erforderlich):

`ash
python -m esptool --chip esp32s3 --port COM3 --baud 921600 write_flash 0x0 merged_firmware.bin
`
*(Ersetze COM3 durch den tatsächlichen COM-Port deines Geräts).*

---

## 📋 Funktionsübersicht & Relais-Belegung

* **RO 1 / RO 2:** SG-Ready Wärmepumpen-Kontakte (Zustände 1 bis 4: Sperre, Normal, PV-Empfehlung, Maximallast)
* **RO 3:** Wallbox Freigabekontakt / Ladesteuerung
* **RO 4 – RO 6:** 3-stufiger Heizstab für PV-Überschuss
* **DI 1 – DI 4:** § 14a EnWG VNB Steuerbox-Signale (100% / 60% / 30% / 0% Dimmung)
* **RS485:** Modbus RTU für Stromzähler (Eastron SDM630 / Sungrow / Janitza)
* **CAN-Bus:** 500 kBit/s Batterie-Telemetrie (Pylontech / BYD)

---

## 📦 Versions-Historie (Changelog)

### Version 1.0.0 (2026-09-24)
* Initiales Release der Industrie-Firmware für Waveshare ESP32-S3 Industrial PoE 8DI/8RO.
* Vollständige § 14a EnWG Auswertung mit prioritärem Hardware-Override (< 50ms Reaktionszeit).
* Lokales Web-Dashboard zur Konfiguration, Signal-Überwachung und für lokale Web-Updates.
* Remote-OTA Unterstützung für vollautomatische Updates aus dem Moniy Management.
