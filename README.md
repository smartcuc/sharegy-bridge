# ? Sharegy Hardware Bridge - Firmware & Update-Center

Offizielles Firmware-Release-Repository für die **Sharegy DIN-Rail Hardware Bridge** (Hutschienen-Gateway für SG-Ready, § 14a EnWG Steuerboxen, RS485 Modbus und CAN-Bus).

* **Ziel-Hardware:** Waveshare ESP32-S3 Industrial PoE 8DI / 8RO Controller
* **Aktuelle Version:** 2.2.0 (Stable)
* **Download:** ?? [**sharegy_bridge_latest.bin**](./sharegy_bridge_latest.bin)
* **Komplettflash (0x0):** ?? [**merged_firmware.bin**](./merged_firmware.bin)

---

## ? 1-Klick Cloud-Update (Standard)

Ab Firmware 2.2.0 ist das **1-Klick Cloud-Update** der Standard:

1. Öffne die Web-UI deiner Bridge (**http://sharegy-bridge.local** oder lokale IP).
2. Die Bridge erkennt neue Versionen automatisch.
3. Klicke einfach auf **"? 1-Klick Update"** – die Firmware wird geladen, geflasht und die Bridge startet neu.

---

## ?? Manuelles Update via Web-Upload (Option)

1. Lade [**sharegy_bridge_latest.bin**](./sharegy_bridge_latest.bin) oder [**sharegy_bridge_v2.2.0.bin**](./sharegy_bridge_v2.2.0.bin) herunter.
2. Öffne die Web-UI der Bridge.
3. Klappe den Bereich *"?? Manuelles Datei-Upload (.bin) als Option"* auf, wähle die Datei und klicke auf Hochladen.

---

## ?? Versions-Historie (Changelog)

### Version 2.2.0 (2026-09-24)
* **1-Klick Cloud OTA:** Automatische Erkennung neuer Releases via `version.json` und 1-Klick Flash per HTTPS direkt aus der Web-UI.
* **Moniy Flotten-Integration:** Robuster Application-Level Heartbeat ohne Disconnect-Flapping.
* **Standardisierung:** Naming auf `sharegy-bridge` harmonisiert.

### Version 1.0.0 (2026-09-24)
* Initiales Release mit § 14a EnWG, 8x RO, 8x DI, SG-Ready Matrix und manuellem Web-Upload.

