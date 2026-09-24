# ⚡ Waveshare ESP32-S3 Flash- & Inbetriebnahmeanleitung

Diese Schritt-für-Schritt-Anleitung führt dich vom Source-Code bis zum erfolgreichen Flashen der Firmware auf deine Waveshare ESP32-S3 Hardware Bridge (8DI / 8RO / RS485 / CAN / PoE).

---

## 1. Voraussetzungen

* **Hardware:** Waveshare ESP32-S3 Industrial Board.
* **Kabel:** 1x USB-C Datenkabel (kein reines Ladekabel!).
* **PC:** Windows mit installiertem VS Code oder Python.

---

## 2. Weg 1: Über VS Code & PlatformIO (Empfohlen)

### Schritt 1: PlatformIO IDE in VS Code installieren
1. Öffne **VS Code**.
2. Klicke links auf das **Erweiterungs-Symbol** (oder drücke `Strg + Umschalt + X`).
3. Suche nach **`PlatformIO IDE`** und klicke auf **Installieren**.
4. Warte ca. 1–2 Minuten, bis PlatformIO im Hintergrund initialisiert wurde.

### Schritt 2: Firmware-Projekt öffnen
1. Klicke in VS Code oben auf **Datei ➔ Ordner öffnen...**
2. Wähle den Ordner:
   ```
   c:\Users\Public\Dev\sharegy\firmware\waveshare_esp32s3_bridge
   ```
3. PlatformIO erkennt das Projekt automatisch anhand der `platformio.ini` und lädt alle nötigen Toolchains und Bibliotheken (ArduinoJson, PubSubClient, Modbus) herunter.

### Schritt 3: Board anschließen
1. Verbinde das Waveshare Board per USB-C mit deinem PC.
2. Windows weist dem Board automatisch einen COM-Port zu (z.B. `COM3` oder `COM5`).

### Schritt 4: Build & Upload (1-Klick)
In der blauen Statusleiste ganz unten in VS Code findest du die PlatformIO-Symbole:
1. **`✓` (Build):** Kompiliert das Projekt zur Überprüfung.
2. **`➔` (Upload):** Kompiliert und flasht die Firmware automatisch auf das angeschlossene Board.
3. **`🔌` (Serial Monitor):** Öffnet den seriellen Monitor bei 115200 Baud zur Echtzeit-Ausgabe.

---

## 3. Weg 2: Schnell über das Terminal (CLI)

Falls du lieber direkt in der Konsole arbeitest:

1. **PlatformIO Core installieren:**
   ```powershell
   pip install platformio
   ```

2. **In das Projektverzeichnis wechseln:**
   ```powershell
   cd c:\Users\Public\Dev\sharegy\firmware\waveshare_esp32s3_bridge
   ```

3. **Flashen:**
   ```powershell
   pio run -t upload
   ```

4. **Seriellen Monitor starten:**
   ```powershell
   pio device monitor
   ```

---

## 4. Wichtige Hardware-Tipps für den ESP32-S3

### A. Der ESP32-S3 Bootloader-Modus
Sollte PlatformIO beim Upload melden: `A fatal error occurred: Failed to connect to ESP32-S3`:
1. Halte die **`BOOT`**-Taste auf dem Board gedrückt.
2. Drücke kurz die **`RESET`** (oder `EN`)-Taste.
3. Lass die **`BOOT`**-Taste los.
4. Starte den Upload erneut – der ESP32-S3 ist jetzt im Bootloader-Download-Modus.

### B. Serieller Test-Befehlssatz
Sobald der serielle Monitor (115200 Baud) geöffnet ist, kannst du folgende Testbefehle per Tastatur eingeben:

| Befehl | Auswirkung |
| :--- | :--- |
| `PING` | Antwortet mit `PONG` (Heartbeat) |
| `SET_RELAY 1 1` | Schaltet Relais 1 EIN (z.B. Klemme 1 Wärmepumpe) |
| `SET_RELAY 1 0` | Schaltet Relais 1 AUS |
| `SET_RELAY 3 1` | Schaltet Relais 3 EIN (Wallbox-Freigabe) |
| `SG_READY 3` | Schaltet SG-Ready in Modus 3 (PV-Überschuss-Booster) |
| `SG_READY 1` | Schaltet SG-Ready in Modus 1 (Netzsperre / 0%) |
| `SG_READY 2` | Schaltet SG-Ready in Modus 2 (Normalbetrieb) |
