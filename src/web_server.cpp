#include "web_server.h"
#include "relays.h"
#include "inputs.h"
#include "rpc_dispatcher.h"
#include "ota_manager.h"
#include "config.h"
#include "addon_manager.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include <HTTPClient.h>

DeviceWebServer webServer;
static Preferences prefs;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>sharegy Hardware Bridge</title>
    <style>
        :root { --bg: #090d16; --card: #131b2e; --border: #1e293b; --accent: #10b981; --text: #f8fafc; --muted: #94a3b8; --indigo: #6366f1; --danger: #ef4444; }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
        body { background: var(--bg); color: var(--text); padding: 16px; max-width: 900px; margin: 0 auto; }
        header { display: flex; align-items: center; justify-content: space-between; border-bottom: 1px solid var(--border); padding-bottom: 16px; margin-bottom: 20px; flex-wrap: wrap; gap: 10px; }
        .logo { font-size: 1.25rem; font-weight: 900; display: flex; align-items: center; gap: 8px; }
        .badges { display: flex; gap: 6px; flex-wrap: wrap; }
        .badge { font-size: 0.72rem; padding: 4px 8px; border-radius: 9999px; font-weight: bold; border: 1px solid transparent; }
        .badge.green { background: rgba(16,185,129,0.2); color: #34d399; border-color: rgba(16,185,129,0.4); }
        .badge.indigo { background: rgba(99,102,241,0.2); color: #a5b4fc; border-color: rgba(99,102,241,0.4); }
        .badge.gray { background: rgba(148,163,184,0.1); color: #94a3b8; border-color: #334155; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(340px, 1fr)); gap: 16px; margin-bottom: 20px; }
        .card { background: var(--card); border: 1px solid var(--border); border-radius: 16px; padding: 18px; }
        .card h2 { font-size: 1rem; margin-bottom: 12px; display: flex; align-items: center; gap: 8px; color: #f1f5f9; }
        .relays { display: grid; grid-template-columns: repeat(4, 1fr); gap: 8px; }
        .btn-relay { background: #1e293b; border: 1px solid #334155; color: var(--text); padding: 12px 6px; border-radius: 10px; font-weight: bold; cursor: pointer; text-align: center; font-size: 0.8rem; transition: 0.2s; }
        .btn-relay.active { background: var(--accent); color: #022c22; border-color: #34d399; box-shadow: 0 0 12px rgba(16,185,129,0.4); }
        .sg-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-top: 10px; }
        .btn-sg { background: #1e293b; border: 1px solid #334155; color: var(--text); padding: 10px; border-radius: 8px; font-size: 0.75rem; font-weight: bold; cursor: pointer; text-align: left; }
        .inputs-list { display: flex; flex-direction: column; gap: 6px; }
        .input-row { display: flex; align-items: center; justify-content: space-between; font-size: 0.8rem; padding: 6px 10px; background: #0f172a; border-radius: 8px; }
        .led { width: 10px; height: 10px; border-radius: 50%; background: #334155; }
        .led.on { background: #38bdf8; box-shadow: 0 0 8px #38bdf8; }
        .led.warn { background: #f59e0b; box-shadow: 0 0 8px #f59e0b; }
        .led.danger { background: #ef4444; box-shadow: 0 0 8px #ef4444; }
        label { font-size:0.75rem; color:var(--muted); display: block; margin-top: 6px; }
        input, select { width: 100%; background: #0f172a; border: 1px solid #334155; color: white; padding: 10px; border-radius: 8px; margin-top: 4px; margin-bottom: 8px; font-size: 0.85rem; }
        .checkbox-row { display: flex; align-items: center; gap: 8px; margin: 10px 0; font-size: 0.85rem; }
        .checkbox-row input { width: auto; margin: 0; }
        .pw-wrapper { position: relative; width: 100%; margin-top: 4px; margin-bottom: 8px; }
        .pw-wrapper input { margin: 0; padding-right: 44px; }
        .pw-toggle { position: absolute; right: 8px; top: 50%; transform: translateY(-50%); background: transparent; border: none; color: var(--muted); cursor: pointer; font-size: 1.1rem; padding: 4px; }
        .pin-input { font-size: 1.5rem !important; text-align: center; letter-spacing: 6px; font-family: monospace; font-weight: 900; color: #38bdf8 !important; border-color: #3b82f6 !important; }
        .btn-primary { width: 100%; background: #3b82f6; border: none; color: white; padding: 12px; border-radius: 10px; font-weight: bold; cursor: pointer; margin-top: 8px; transition: 0.2s; }
        .btn-primary:hover { background: #2563eb; }
        .btn-secondary { background: #334155; border: 1px solid #475569; color: white; padding: 8px 12px; border-radius: 8px; font-size: 0.8rem; font-weight: bold; cursor: pointer; display: inline-flex; align-items: center; gap: 6px; }
        .btn-success { width: 100%; background: #10b981; border: none; color: #022c22; padding: 12px; border-radius: 10px; font-weight: bold; cursor: pointer; margin-top: 8px; }
        .alert-box { padding: 12px; border-radius: 10px; font-size: 0.85rem; margin-top: 10px; display: none; line-height: 1.4; }
        .alert-box.success { background: rgba(16,185,129,0.15); border: 1px solid #10b981; color: #34d399; display: block; }
        .alert-box.error { background: rgba(239,68,68,0.15); border: 1px solid #ef4444; color: #f87171; display: block; }
        .alert-box.info { background: rgba(59,130,246,0.15); border: 1px solid #3b82f6; color: #60a5fa; display: block; }
        .telemetry-val { font-family: monospace; font-weight: bold; color: #38bdf8; }
    </style>
</head>
<body>
    <header>
        <div class="logo">⚡ sharegy <span class="badge green" id="pairBadge">Bridge Online</span></div>
        <div class="badges">
            <span class="badge gray" id="ethBadge">LAN: Getrennt</span>
            <span class="badge gray" id="wifiBadge">WLAN: Getrennt</span>
            <span class="badge indigo" id="hostBadge">Host: sharegy-bridge</span>
        </div>
    </header>

    <!-- 🔑 30-Sekunden Zero-Touch PIN Kopplung -->
    <div class="card" style="margin-bottom: 16px; border-color: rgba(99,102,241,0.4); background: linear-gradient(180deg, #18223d, #131b2e);">
        <h2>🔑 1-Klick Sharegy Account-Kopplung</h2>
        <p style="font-size:0.8rem; color:var(--muted); margin-bottom:10px;">
            Generiere in deiner Sharegy-App einen 6-stelligen Kopplungscode und tippe ihn hier ein:
        </p>
        <form onsubmit="claimPairingPin(event)">
            <input type="text" id="pairingPin" class="pin-input" placeholder="482-913" maxlength="7" required>
            <button type="submit" class="btn-success" id="btnPair">🔗 Gerät mit Sharegy-Konto verknüpfen</button>
        </form>
    </div>

    <div class="grid">
        <!-- ⚙️ SYSTEM & SICHERHEIT -->
        <div class="card">
            <h2>🛡️ System & Sicherheit</h2>
            <form onsubmit="saveSystemConfig(event)">
                <label>Hostname / mDNS Domain (z.B. sharegy-bridge-c2f1e8)</label>
                <input type="text" id="sysHostName" placeholder="sharegy-bridge-c2f1e8" required>

                <div class="checkbox-row">
                    <input type="checkbox" id="sysAuthEn" checked onchange="toggleAuthFields()">
                    <label style="margin:0; cursor:pointer;" for="sysAuthEn">Web-UI Passwortschutz aktivieren</label>
                </div>

                <div id="authFields">
                    <label>Web Benutzername</label>
                    <input type="text" id="sysUser" value="admin">
                    <label>Web Passwort</label>
                    <div class="pw-wrapper">
                        <input type="password" id="sysPass" placeholder="Neues Passwort">
                        <button type="button" class="pw-toggle" onclick="togglePasswordVisibility('sysPass', this)">👁️</button>
                    </div>
                </div>

                <div class="checkbox-row" style="border-top:1px solid #1e293b; padding-top:8px;">
                    <input type="checkbox" id="sysWifiEn" checked>
                    <label style="margin:0; cursor:pointer;" for="sysWifiEn">WLAN Funkmodul aktivieren</label>
                </div>

                <button type="submit" class="btn-primary">💾 System-Einstellungen speichern</button>
                <div id="sysAlert" class="alert-box"></div>
            </form>
        </div>

        <!-- 📶 WLAN SETUP -->
        <div class="card">
            <h2>📶 WLAN & Netzwerk-Setup</h2>
            <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom: 4px;">
                <label>Verfügbare WLAN-Netzwerke</label>
                <button type="button" class="btn-secondary" id="btnScan" onclick="scanWifi()">🔄 Netzwerke suchen</button>
            </div>
            <select id="wifiSsidSelect" onchange="onSsidSelectChange()">
                <option value="">-- Klicke auf "Netzwerke suchen" oder manuell eingeben --</option>
            </select>

            <form onsubmit="testAndSaveWifi(event)">
                <label>WLAN Name (SSID)</label>
                <input type="text" id="wifiSsid" placeholder="z.B. FRITZ!Box 7590" required>

                <label>WLAN Passwort</label>
                <div class="pw-wrapper">
                    <input type="password" id="wifiPass" placeholder="WLAN Passwort eingeben" required>
                    <button type="button" class="pw-toggle" onclick="togglePasswordVisibility('wifiPass', this)">👁️</button>
                </div>

                <button type="submit" class="btn-primary" id="btnSaveWifi">⚡ Verbindung testen & Speichern</button>
                <div id="wifiStatusAlert" class="alert-box"></div>
            </form>
        </div>
    </div>

    <div class="grid">
        <!-- 🔌 ADDONS / MODBUS PROFILE -->
        <div class="card">
            <h2>⚡ Add-ons & Modbus Profile</h2>
            <form onsubmit="saveAddonConfig(event)">
                <div class="checkbox-row">
                    <input type="checkbox" id="addonEn" onchange="toggleAddonFields()">
                    <label style="margin:0; cursor:pointer;" for="addonEn">Add-on Treiber aktivieren</label>
                </div>

                <div id="addonFields">
                    <label>Geräte-Profil / Wechselrichter / Zähler</label>
                    <select id="addonProfile" onchange="onAddonProfileChange()">
                        <option value="1">⚡ Eastron SDM630 / SDM120 (Modbus-RTU RS485)</option>
                        <option value="2">☀️ Sungrow Inverter (RTU RS485 oder Modbus-TCP)</option>
                        <option value="3">☀️ Fronius Symo / Gen24 / Smart Meter (SunSpec RTU/TCP)</option>
                        <option value="4">🔋 Deye / SunSynk Hybrid-Wechselrichter (RS485 RTU)</option>
                        <option value="5">☀️ SMA Sunny Tripower / Boy (Modbus-TCP SunSpec)</option>
                        <option value="6">🔋 Victron Energy GX / MultiPlus (Modbus-TCP)</option>
                        <option value="7">🌡️ SG-Ready Wärmepumpen Controller (Auto-Relais)</option>
                        <option value="8">🔌 S0-Impulszähler (Wirkleistung & kWh Berechnung)</option>
                    </select>

                    <label>Übertragungsprotokoll</label>
                    <select id="addonProto" onchange="onAddonProtoChange()">
                        <option value="1">RS485 Klemmen A/B (Modbus-RTU)</option>
                        <option value="2">Ethernet / WLAN Netzwerk (Modbus-TCP)</option>
                    </select>

                    <div id="rtuFields">
                        <div style="display:grid; grid-template-columns: 1fr 1fr; gap:8px;">
                            <div>
                                <label>Modbus Slave-ID</label>
                                <input type="number" id="addonSlaveId" value="1" min="1" max="247">
                            </div>
                            <div>
                                <label>Baudrate</label>
                                <select id="addonBaud">
                                    <option value="9600">9600 Baud</option>
                                    <option value="19200">19200 Baud</option>
                                    <option value="115200">115200 Baud</option>
                                </select>
                            </div>
                        </div>
                    </div>

                    <div id="tcpFields" style="display:none;">
                        <div style="display:grid; grid-template-columns: 2fr 1fr; gap:8px;">
                            <div>
                                <label>Wechselrichter IP-Adresse</label>
                                <input type="text" id="addonTcpHost" placeholder="192.168.1.150">
                            </div>
                            <div>
                                <label>Port</label>
                                <input type="number" id="addonTcpPort" value="502">
                            </div>
                        </div>
                    </div>

                    <div id="s0Fields" style="display:none;">
                        <label>S0 Impulse pro kWh (z.B. 1000 oder 2000)</label>
                        <input type="number" id="addonS0Imp" value="1000">
                    </div>
                </div>

                <button type="submit" class="btn-primary">💾 Add-on Konfiguration speichern</button>
                <div id="addonAlert" class="alert-box"></div>
            </form>
        </div>

        <!-- 📡 MQTT / MQTTS CONFIG -->
        <div class="card">
            <h2>📡 Lokales MQTT / MQTTS (TLS)</h2>
            <form onsubmit="saveMqttConfig(event)">
                <div style="display:grid; grid-template-columns: 2fr 1fr; gap:8px;">
                    <div>
                        <label>MQTT Broker Hostname / IP</label>
                        <input type="text" id="mqttHost" placeholder="192.168.1.100">
                    </div>
                    <div>
                        <label>Port</label>
                        <input type="number" id="mqttPort" value="1883">
                    </div>
                </div>

                <div class="checkbox-row">
                    <input type="checkbox" id="mqttTls" onchange="onMqttTlsChange()">
                    <label style="margin:0; cursor:pointer;" for="mqttTls">MQTTS (TLS-Verschlüsselung aktivieren)</label>
                </div>

                <div style="display:grid; grid-template-columns: 1fr 1fr; gap:8px;">
                    <div>
                        <label>Benutzername</label>
                        <input type="text" id="mqttUser" placeholder="Optional">
                    </div>
                    <div>
                        <label>Passwort</label>
                        <input type="password" id="mqttPass" placeholder="Optional">
                    </div>
                </div>

                <label>Topic-Präfix</label>
                <input type="text" id="mqttPrefix" value="sharegy">

                <button type="submit" class="btn-primary">💾 MQTT Einstellungen speichern</button>
                <div id="mqttAlert" class="alert-box"></div>
            </form>
        </div>
    </div>

    <div class="grid">
        <!-- Relays -->
        <div class="card">
            <h2>🔌 8x Relais Ausgänge (DO)</h2>
            <div class="relays" id="relayContainer"></div>

            <h2 style="margin-top:16px;">🌡️ SG-Ready Wärmepumpen Matrix</h2>
            <div class="sg-grid">
                <button class="btn-sg" onclick="setSgReady(1)">⛔ 1: Sperre (0%)</button>
                <button class="btn-sg" onclick="setSgReady(2)">🟢 2: Normal (Auto)</button>
                <button class="btn-sg" onclick="setSgReady(3)">☀️ 3: PV-Überschuss</button>
                <button class="btn-sg" onclick="setSgReady(4)">⚡ 4: Maximallast</button>
            </div>
        </div>

        <!-- Inputs / Grid 14a -->
        <div class="card">
            <h2>📥 § 14a EnWG & Digitaleingänge</h2>
            <div class="inputs-list">
                <div class="input-row"><span>DI 1: Normalbetrieb (100 %)</span><div class="led" id="di1"></div></div>
                <div class="input-row"><span>DI 2: Vorwarnung (60 %)</span><div class="led" id="di2"></div></div>
                <div class="input-row"><span>DI 3: § 14a Dimmung (30 %)</span><div class="led" id="di3"></div></div>
                <div class="input-row"><span>DI 4: Netzsperre / Notaus (0 %)</span><div class="led" id="di4"></div></div>
                <div class="input-row"><span>DI 5: S0-Zähler Impulse</span><span id="s0Count" class="telemetry-val">0</span></div>
            </div>

            <h2 style="margin-top:16px;">🔄 Firmware Over-The-Air (OTA)</h2>
            <form method="POST" action="/update" enctype="multipart/form-data">
                <input type="file" name="update" accept=".bin" required>
                <button type="submit" class="btn-primary" style="background:#475569">⬆️ Neue Firmware installieren</button>
            </form>
        </div>
    </div>

    <script>
        async function fetchStatus() {
            try {
                const res = await fetch('/api/status');
                const data = await res.json();
                
                document.getElementById('hostBadge').innerText = 'Host: ' + (data.host_name || 'sharegy-bridge');
                
                // Ethernet Badge
                const ethBadge = document.getElementById('ethBadge');
                if (data.eth_connected) {
                    ethBadge.className = 'badge green';
                    ethBadge.innerText = 'LAN: ' + data.eth_ip;
                } else {
                    ethBadge.className = 'badge gray';
                    ethBadge.innerText = 'LAN: Getrennt';
                }

                // WiFi Badge
                const wifiBadge = document.getElementById('wifiBadge');
                if (data.wifi_connected) {
                    wifiBadge.className = 'badge green';
                    wifiBadge.innerText = 'WLAN: ' + data.wifi_ip + ' (' + data.wifi_rssi + ' dBm)';
                } else if (!data.wifi_enabled) {
                    wifiBadge.className = 'badge gray';
                    wifiBadge.innerText = 'WLAN: Deaktiviert';
                } else {
                    wifiBadge.className = 'badge gray';
                    wifiBadge.innerText = 'WLAN: Nicht verbunden';
                }

                if (data.paired) {
                    const badge = document.getElementById('pairBadge');
                    badge.innerText = 'Gekoppelt mit Sharegy';
                    badge.className = 'badge indigo';
                }

                // Update Relays
                let rHtml = '';
                for (let i = 1; i <= 8; i++) {
                    const active = data.relays['ro' + i];
                    rHtml += `<button class="btn-relay ${active ? 'active' : ''}" onclick="toggleRelay(${i}, ${!active})">RO ${i}<br>${active ? 'EIN' : 'AUS'}</button>`;
                }
                document.getElementById('relayContainer').innerHTML = rHtml;

                // Update LEDs
                document.getElementById('di1').className = 'led ' + (data.inputs.di1 ? 'on' : '');
                document.getElementById('di2').className = 'led ' + (data.inputs.di2 ? 'warn' : '');
                document.getElementById('di3').className = 'led ' + (data.inputs.di3 ? 'warn' : '');
                document.getElementById('di4').className = 'led ' + (data.inputs.di4 ? 'danger' : '');
                document.getElementById('s0Count').innerText = data.s0_pulses || 0;
            } catch(e) { console.error(e); }
        }

        async function toggleRelay(ch, state) {
            await fetch('/api/relay', { method: 'POST', headers: {'Content-Type':'application/json'}, body: JSON.stringify({channel: ch, state: state}) });
            fetchStatus();
        }

        async function setSgReady(mode) {
            await fetch('/api/sg-ready', { method: 'POST', headers: {'Content-Type':'application/json'}, body: JSON.stringify({mode: mode}) });
            fetchStatus();
        }

        function togglePasswordVisibility(id, btn) {
            const el = document.getElementById(id);
            if (el.type === 'password') {
                el.type = 'text';
                btn.innerText = '🙈';
            } else {
                el.type = 'password';
                btn.innerText = '👁️';
            }
        }

        function toggleAuthFields() {
            document.getElementById('authFields').style.display = document.getElementById('sysAuthEn').checked ? 'block' : 'none';
        }

        function toggleAddonFields() {
            document.getElementById('addonFields').style.display = document.getElementById('addonEn').checked ? 'block' : 'none';
        }

        function onAddonProtoChange() {
            const proto = document.getElementById('addonProto').value;
            document.getElementById('rtuFields').style.display = (proto === '1') ? 'block' : 'none';
            document.getElementById('tcpFields').style.display = (proto === '2') ? 'block' : 'none';
        }

        function onAddonProfileChange() {
            const prof = document.getElementById('addonProfile').value;
            document.getElementById('s0Fields').style.display = (prof === '8') ? 'block' : 'none';
        }

        function onMqttTlsChange() {
            const isTls = document.getElementById('mqttTls').checked;
            const port = document.getElementById('mqttPort');
            if (isTls && port.value === '1883') port.value = '8883';
            if (!isTls && port.value === '8883') port.value = '1883';
        }

        async function scanWifi() {
            const btn = document.getElementById('btnScan');
            const select = document.getElementById('wifiSsidSelect');
            btn.innerText = '⏳ Scanne...';
            btn.disabled = true;

            try {
                const res = await fetch('/api/wifi/scan');
                const data = await res.json();
                select.innerHTML = '<option value="">-- Wähle dein WLAN aus --</option>';
                
                if (data.networks && data.networks.length > 0) {
                    data.networks.forEach(net => {
                        if (net.ssid && net.ssid.length > 0) {
                            const opt = document.createElement('option');
                            opt.value = net.ssid;
                            const lock = net.secure ? '🔒' : '🔓';
                            opt.innerText = `${lock} ${net.ssid} (${net.rssi} dBm)`;
                            select.appendChild(opt);
                        }
                    });
                } else {
                    select.innerHTML = '<option value="">Keine WLAN-Netzwerke gefunden</option>';
                }
            } catch (err) {
                alert('Fehler beim Scannen nach WLAN-Netzwerken.');
            } finally {
                btn.innerText = '🔄 Netzwerke suchen';
                btn.disabled = false;
            }
        }

        function onSsidSelectChange() {
            const select = document.getElementById('wifiSsidSelect');
            if (select.value) {
                document.getElementById('wifiSsid').value = select.value;
            }
        }

        async function testAndSaveWifi(e) {
            e.preventDefault();
            const ssid = document.getElementById('wifiSsid').value.trim();
            const pass = document.getElementById('wifiPass').value;
            const btn = document.getElementById('btnSaveWifi');
            const alertBox = document.getElementById('wifiStatusAlert');

            btn.disabled = true;
            btn.innerText = '⏳ Teste Verbindung zu "' + ssid + '"...';
            alertBox.className = 'alert-box info';
            alertBox.innerText = 'Verbinde mit "' + ssid + '"... Bitte ca. 5–8 Sekunden warten.';

            try {
                const res = await fetch('/api/wifi/test', {
                    method: 'POST',
                    headers: {'Content-Type':'application/json'},
                    body: JSON.stringify({ ssid: ssid, pass: pass })
                });
                const data = await res.json();

                if (data.status === 'SUCCESS') {
                    alertBox.className = 'alert-box success';
                    alertBox.innerHTML = '<strong>✅ Verbindung erfolgreich!</strong><br>Zugeordnete IP: <strong>' + data.ip + '</strong><br>Neustart im Client-Modus...';
                    btn.innerText = '✓ Erfolgreich! Neustart...';
                } else {
                    alertBox.className = 'alert-box error';
                    alertBox.innerHTML = '<strong>❌ Verbindung fehlgeschlagen:</strong><br>' + (data.message || 'Passwort falsch oder Signal zu schwach.');
                    btn.innerText = '⚡ Verbindung testen & Speichern';
                    btn.disabled = false;
                }
            } catch (err) {
                alertBox.className = 'alert-box error';
                alertBox.innerText = 'Netzwerkfehler beim Verbindungstest.';
                btn.innerText = '⚡ Verbindung testen & Speichern';
                btn.disabled = false;
            }
        }

        async function saveSystemConfig(e) {
            e.preventDefault();
            const payload = {
                host_name: document.getElementById('sysHostName').value.trim(),
                auth_en: document.getElementById('sysAuthEn').checked,
                web_user: document.getElementById('sysUser').value.trim(),
                web_pass: document.getElementById('sysPass').value,
                wifi_en: document.getElementById('sysWifiEn').checked
            };
            const alertBox = document.getElementById('sysAlert');
            try {
                const res = await fetch('/api/system', {
                    method: 'POST',
                    headers: {'Content-Type':'application/json'},
                    body: JSON.stringify(payload)
                });
                alertBox.className = 'alert-box success';
                alertBox.innerText = '✅ System-Einstellungen gespeichert! Startet neu...';
            } catch(err) {
                alertBox.className = 'alert-box error';
                alertBox.innerText = 'Fehler beim Speichern.';
            }
        }

        async function saveAddonConfig(e) {
            e.preventDefault();
            const payload = {
                enabled: document.getElementById('addonEn').checked,
                profile: parseInt(document.getElementById('addonProfile').value),
                proto: parseInt(document.getElementById('addonProto').value),
                slave_id: parseInt(document.getElementById('addonSlaveId').value),
                baud: parseInt(document.getElementById('addonBaud').value),
                tcp_host: document.getElementById('addonTcpHost').value.trim(),
                tcp_port: parseInt(document.getElementById('addonTcpPort').value),
                s0_imp: parseInt(document.getElementById('addonS0Imp').value)
            };
            const alertBox = document.getElementById('addonAlert');
            try {
                await fetch('/api/addon', {
                    method: 'POST',
                    headers: {'Content-Type':'application/json'},
                    body: JSON.stringify(payload)
                });
                alertBox.className = 'alert-box success';
                alertBox.innerText = '✅ Add-on Treiber aktiv und gespeichert!';
            } catch(err) {
                alertBox.className = 'alert-box error';
                alertBox.innerText = 'Fehler beim Speichern des Add-ons.';
            }
        }

        async function saveMqttConfig(e) {
            e.preventDefault();
            const payload = {
                host: document.getElementById('mqttHost').value.trim(),
                port: parseInt(document.getElementById('mqttPort').value),
                tls: document.getElementById('mqttTls').checked,
                user: document.getElementById('mqttUser').value.trim(),
                pass: document.getElementById('mqttPass').value,
                prefix: document.getElementById('mqttPrefix').value.trim()
            };
            const alertBox = document.getElementById('mqttAlert');
            try {
                await fetch('/api/mqtt', {
                    method: 'POST',
                    headers: {'Content-Type':'application/json'},
                    body: JSON.stringify(payload)
                });
                alertBox.className = 'alert-box success';
                alertBox.innerText = '✅ MQTT-Einstellungen gespeichert!';
            } catch(err) {
                alertBox.className = 'alert-box error';
                alertBox.innerText = 'Fehler beim Speichern von MQTT.';
            }
        }

        async function claimPairingPin(e) {
            e.preventDefault();
            const pin = document.getElementById('pairingPin').value.replace('-','').trim();
            const btn = document.getElementById('btnPair');
            btn.innerText = 'Verbinde mit Sharegy Cloud...';
            
            try {
                const res = await fetch('/api/pair', { method: 'POST', headers: {'Content-Type':'application/json'}, body: JSON.stringify({pin: pin}) });
                const data = await res.json();
                if (data.status === 'PAIRED') {
                    alert('🎉 Erfolgreich gekoppelt mit: ' + data.device_name);
                    fetchStatus();
                } else {
                    alert('Fehler: ' + (data.error || 'PIN ungültig oder abgelaufen.'));
                }
            } catch(err) {
                alert('Verbindungsfehler zur Cloud.');
            }
            btn.innerText = '🔗 Gerät mit Sharegy-Konto verknüpfen';
        }

        setInterval(fetchStatus, 2000);
        fetchStatus();
    </script>
</body>
</html>
)rawliteral";

void DeviceWebServer::init(bool isApMode) {
    isCaptivePortal = isApMode;
    prefs.begin("sharegy_cfg", false);
    authEnabled = prefs.getBool("auth_en", true);
    authUser = prefs.getString("web_user", DEFAULT_WEB_USER);
    authPass = prefs.getString("web_pass", DEFAULT_WEB_PASS);

    if (isCaptivePortal) {
        dnsServer.start(53, "*", WiFi.softAPIP());
    }

    ota.init(&server);
    setupRoutes();
    server.begin();
    Serial.printf("[WEB] Webserver started on port 80 (Auth: %s)\n", authEnabled ? "ENABLED" : "DISABLED");
}

bool DeviceWebServer::checkAuth() {
    if (!authEnabled || isCaptivePortal) return true;
    if (!server.authenticate(authUser.c_str(), authPass.c_str())) {
        server.requestAuthentication();
        return false;
    }
    return true;
}

void DeviceWebServer::setupRoutes() {
    server.on("/", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        handleRoot();
    });

    server.on("/api/status", HTTP_GET, [this]() {
        handleApiStatus();
    });

    server.on("/api/relay", HTTP_POST, [this]() {
        if (!checkAuth()) return;
        handleApiRelay();
    });

    server.on("/api/sg-ready", HTTP_POST, [this]() {
        if (!checkAuth()) return;
        handleApiSgReady();
    });

    server.on("/api/rpc", HTTP_POST, [this]() {
        handleApiRpc();
    });

    server.on("/api/system", HTTP_POST, [this]() {
        if (!checkAuth()) return;
        handleApiSystem();
    });

    server.on("/api/mqtt", HTTP_POST, [this]() {
        if (!checkAuth()) return;
        handleApiMqtt();
    });

    server.on("/api/addon", HTTP_POST, [this]() {
        if (!checkAuth()) return;
        handleApiAddon();
    });

    server.on("/api/wifi/scan", HTTP_GET, [this]() {
        handleApiWifiScan();
    });

    server.on("/api/wifi/test", HTTP_POST, [this]() {
        handleApiWifiTest();
    });

    // 🔑 6-Digit Pairing Endpoint
    server.on("/api/pair", HTTP_POST, [this]() {
        if (!server.hasArg("plain")) {
            server.send(400, "application/json", "{\"error\":\"Missing body\"}");
            return;
        }
        JsonDocument req;
        deserializeJson(req, server.arg("plain"));
        const char* pin = req["pin"] | "";

        HTTPClient http;
        http.begin("https://api.sharegy.de/api/devices/pairing/claim/");
        http.addHeader("Content-Type", "application/json");

        JsonDocument claimPayload;
        claimPayload["pin"] = pin;
        claimPayload["device_id"] = deviceId;
        claimPayload["mac"] = WiFi.macAddress();
        claimPayload["model"] = "Waveshare ESP32-S3 PoE 8DI/8RO";
        claimPayload["firmware_version"] = "1.1.0";

        String payloadStr;
        serializeJson(claimPayload, payloadStr);

        int httpCode = http.POST(payloadStr);
        if (httpCode == 200) {
            String respStr = http.getString();
            JsonDocument respDoc;
            deserializeJson(respDoc, respStr);

            prefs.putString("dev_token", respDoc["device_token"] | "");
            prefs.putString("dev_topic", respDoc["mqtt_topic"] | "");
            prefs.putBool("is_paired", true);

            server.send(200, "application/json", respStr);
        } else {
            server.send(httpCode > 0 ? httpCode : 500, "application/json", "{\"error\":\"Pairing fehlgeschlagen oder PIN abgelaufen\"}");
        }
        http.end();
    });

    server.onNotFound([this]() { handleNotFound(); });
}

void DeviceWebServer::handleRoot() {
    server.send(200, "text/html", INDEX_HTML);
}

void DeviceWebServer::handleApiStatus() {
    JsonDocument doc;
    doc["uptime_s"] = millis() / 1000;
    doc["device_id"] = deviceId;
    doc["host_name"] = hostName;
    doc["paired"] = prefs.getBool("is_paired", false);
    doc["wifi_enabled"] = wifiRadioEnabled;
    doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
    doc["wifi_ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "";
    doc["wifi_rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
    doc["eth_connected"] = ethConnected;
    doc["eth_ip"] = ethIp;
    
    JsonObject r = doc["relays"].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "ro%d", i + 1);
        r[key] = relays.getRelay(i);
    }

    JsonObject in = doc["inputs"].to<JsonObject>();
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "di%d", i + 1);
        in[key] = inputs.getInput(i);
    }

    GridControlSignals grid = inputs.getGridStatus();
    doc["s0_pulses"] = grid.s0PulseCount;

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void DeviceWebServer::handleApiRelay() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    int ch = doc["channel"] | 0;
    bool st = doc["state"] | false;
    if (ch >= 1 && ch <= 8) {
        relays.setRelay(ch - 1, st);
        server.send(200, "application/json", "{\"status\":\"OK\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid channel\"}");
    }
}

void DeviceWebServer::handleApiSgReady() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    int mode = doc["mode"] | 2;
    if (mode >= 1 && mode <= 4) {
        relays.setSgReadyMode((SgReadyState)mode);
        server.send(200, "application/json", "{\"status\":\"OK\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid mode\"}");
    }
}

void DeviceWebServer::handleApiRpc() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }
    String resp = rpc.handleJsonRpc(server.arg("plain"));
    server.send(200, "application/json", resp);
}

void DeviceWebServer::handleApiSystem() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    const char* host = doc["host_name"] | "";
    bool authEn = doc["auth_en"] | true;
    const char* user = doc["web_user"] | "admin";
    const char* pass = doc["web_pass"] | "";
    bool wifiEn = doc["wifi_en"] | true;

    if (strlen(host) > 0) prefs.putString("host_name", host);
    prefs.putBool("auth_en", authEn);
    if (strlen(user) > 0) prefs.putString("web_user", user);
    if (strlen(pass) > 0) prefs.putString("web_pass", pass);
    prefs.putBool("wifi_en", wifiEn);

    server.send(200, "application/json", "{\"status\":\"SAVED_REBOOTING\"}");
    delay(1000);
    ESP.restart();
}

void DeviceWebServer::handleApiMqtt() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    const char* host = doc["host"] | "";
    int port = doc["port"] | 1883;
    bool tls = doc["tls"] | false;
    const char* user = doc["user"] | "";
    const char* pass = doc["pass"] | "";
    const char* prefix = doc["prefix"] | "sharegy";

    prefs.putString("mqtt_host", host);
    prefs.putInt("mqtt_port", port);
    prefs.putBool("mqtt_tls", tls);
    prefs.putString("mqtt_user", user);
    if (strlen(pass) > 0) prefs.putString("mqtt_pass", pass);
    prefs.putString("mqtt_prefix", prefix);

    server.send(200, "application/json", "{\"status\":\"SAVED\"}");
}

void DeviceWebServer::handleApiAddon() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    AddonConfig cfg;
    cfg.enabled = doc["enabled"] | false;
    cfg.profile = (AddonProfileType)(doc["profile"] | 0);
    cfg.protocol = (ModbusProtocolType)(doc["proto"] | 1);
    cfg.slaveId = (uint8_t)(doc["slave_id"] | 1);
    cfg.baudRate = (uint32_t)(doc["baud"] | 9600);
    const char* tcpHost = doc["tcp_host"] | "";
    strncpy(cfg.tcpHost, tcpHost, sizeof(cfg.tcpHost) - 1);
    cfg.tcpPort = (uint16_t)(doc["tcp_port"] | 502);
    cfg.s0ImpPerKwh = (uint16_t)(doc["s0_imp"] | 1000);

    addonMgr.setProfile(cfg);
    server.send(200, "application/json", "{\"status\":\"SAVED\"}");
}

void DeviceWebServer::handleApiWifiScan() {
    int n = WiFi.scanNetworks(false, true);
    JsonDocument doc;
    JsonArray arr = doc["networks"].to<JsonArray>();
    for (int i = 0; i < n; ++i) {
        JsonObject net = arr.add<JsonObject>();
        net["ssid"] = WiFi.SSID(i);
        net["rssi"] = WiFi.RSSI(i);
        net["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }
    WiFi.scanDelete();
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void DeviceWebServer::handleApiWifiTest() {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    const char* ssid = doc["ssid"] | "";
    const char* pass = doc["pass"] | "";

    if (strlen(ssid) == 0) {
        server.send(400, "application/json", "{\"status\":\"ERROR\",\"message\":\"Bitte eine SSID angeben.\"}");
        return;
    }

    Serial.printf("[NET] Testing WiFi connection to '%s'...\n", ssid);
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);

    unsigned long start = millis();
    bool connected = false;
    while (millis() - start < 8000) {
        delay(300);
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
            break;
        }
    }

    if (connected) {
        String ip = WiFi.localIP().toString();
        Serial.printf("[NET] WiFi Test SUCCESS! IP: %s\n", ip.c_str());

        prefs.putString("wifi_ssid", ssid);
        prefs.putString("wifi_pass", pass);

        JsonDocument resp;
        resp["status"] = "SUCCESS";
        resp["ip"] = ip;
        resp["message"] = "Verbindung erfolgreich! IP: " + ip;
        String respStr;
        serializeJson(resp, respStr);
        server.send(200, "application/json", respStr);

        delay(1500);
        ESP.restart();
    } else {
        Serial.println("[NET] WiFi Test FAILED.");
        WiFi.disconnect(false, false);
        server.send(200, "application/json", "{\"status\":\"ERROR\",\"message\":\"Verbindung fehlgeschlagen! Bitte Passwort und Signalstärke prüfen.\"}");
    }
}

void DeviceWebServer::handleNotFound() {
    if (isCaptivePortal) {
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        server.send(302, "text/plain", "");
    } else {
        server.send(404, "text/plain", "Not Found");
    }
}

void DeviceWebServer::handleClient() {
    if (isCaptivePortal) {
        dnsServer.processNextRequest();
    }
    server.handleClient();
}
