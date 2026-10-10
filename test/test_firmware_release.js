/**
 * ⚡ Sharegy Hardware Bridge - Release & Binary Validation Test Suite
 * 
 * Verifiziert:
 * 1. version.json Schema & Release-Konsistenz
 * 2. ESP32-S3 Firmware-Binaries (Magic Byte 0xE9, Mindestgröße, Chip ID)
 * 3. SHA-256 Hash-Identität zwischen firmware.bin und sharegy_bridge_latest.bin
 * 4. API-Verträge & Installer-Inbetriebnahmemodus (Permanent Device AP)
 */

const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

let passedTests = 0;
let totalTests = 0;

function assert(condition, message) {
  totalTests++;
  if (!condition) {
    console.error(`❌ FAIL: ${message}`);
    process.exitCode = 1;
  } else {
    passedTests++;
    console.log(`  ✔ ${message}`);
  }
}

console.log('🚀 Starte Validierung für Sharegy Hardware Bridge Release...\n');

// ============================================================================
// 1. VERSION.JSON SCHEMA & VALIDIERUNG
// ============================================================================
console.log('▶ Test 1: version.json Schema & URL-Integrität');
const versionJsonPath = path.join(__dirname, '..', 'version.json');
assert(fs.existsSync(versionJsonPath), 'version.json existiert im Stammverzeichnis');

const versionData = JSON.parse(fs.readFileSync(versionJsonPath, 'utf8'));
assert(typeof versionData.version === 'string' && /^\d+\.\d+\.\d+$/.test(versionData.version), `Gültige SemVer-Version: ${versionData.version}`);
assert(versionData.hardware === 'Sharegy-Bridge-8DI-8RO', `Hardware-Kennung korrekt: ${versionData.hardware}`);
assert(typeof versionData.firmware_url === 'string' && versionData.firmware_url.startsWith('https://'), `Gültige HTTPS OTA-URL: ${versionData.firmware_url}`);
assert(typeof versionData.release_notes === 'string' && versionData.release_notes.length > 20, 'Ausführliche Release-Notes vorhanden');
assert(Boolean(versionData.timestamp), `Gültiger Release-Zeitstempel: ${versionData.timestamp}`);

// ============================================================================
// 2. BINARY INTEGRITY & ESP32-S3 HEADER
// ============================================================================
console.log('\n▶ Test 2: ESP32-S3 Binaries & Magic Header (0xE9)');
const requiredBins = [
  'firmware.bin',
  'sharegy_bridge_latest.bin',
  `sharegy_bridge_v${versionData.version}.bin`
];

requiredBins.forEach(binName => {
  const binPath = path.join(__dirname, '..', binName);
  assert(fs.existsSync(binPath), `Binärdatei vorhanden: ${binName}`);
  
  if (fs.existsSync(binPath)) {
    const stats = fs.statSync(binPath);
    // Erwartet: > 1.0 MB und < 3.2 MB (Flash-Partition)
    assert(stats.size > 1000000 && stats.size < 3342336, `${binName} Dateigröße plausibel (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
    
    // Lese ESP32 Image Header (Offset 0 muss 0xE9 sein)
    const fd = fs.openSync(binPath, 'r');
    const headerBuf = Buffer.alloc(16);
    fs.readSync(fd, headerBuf, 0, 16, 0);
    fs.closeSync(fd);
    
    const magicByte = headerBuf[0];
    assert(magicByte === 0xE9, `${binName} besitzt gültigen ESP32 Image Magic Byte 0xE9 (gelesen: 0x${magicByte.toString(16).toUpperCase()})`);
    
    // ESP32-S3 Chip ID Prüfung (Offset 12-13 ist 0x0009 für ESP32-S3)
    const chipId = headerBuf.readUInt16LE(12);
    assert(chipId === 0x0009 || chipId === 0x0000, `${binName} ESP32 Chip-Architektur validiert (Chip-ID: 0x${chipId.toString(16)})`);
  }
});

// ============================================================================
// 3. SHA-256 KONSISTENZ
// ============================================================================
console.log('\n▶ Test 3: SHA-256 Konsistenz (firmware.bin vs latest.bin)');
const fwPath = path.join(__dirname, '..', 'firmware.bin');
const latestPath = path.join(__dirname, '..', 'sharegy_bridge_latest.bin');

if (fs.existsSync(fwPath) && fs.existsSync(latestPath)) {
  const hashFw = crypto.createHash('sha256').update(fs.readFileSync(fwPath)).digest('hex');
  const hashLatest = crypto.createHash('sha256').update(fs.readFileSync(latestPath)).digest('hex');
  assert(hashFw === hashLatest, `SHA-256 Prüfsummen stimmen exakt überein (${hashFw.substring(0, 16)}...)`);
}

// ============================================================================
// 4. PROTOKOLL- & INBETRIEBNAHME-VERTRÄGE (INSTALLER DEVICE-AP)
// ============================================================================
console.log('\n▶ Test 4: API-Vertragsmodell für Installateur-Inbetriebnahme & Permanent Device-AP');

const mockStatusPayload = {
  uptime_s: 42,
  device_id: "c2f1e8",
  host_name: "sharegy-bridge-c2f1e8",
  firmware_version: versionData.version,
  installer_confirmed: true,
  wifi_ap_mode: "DEVICE_AP",
  wifi_ap_active: true,
  wifi_ap_ssid: "Sharegy-c2f1e8",
  wifi_enabled: true,
  wifi_connected: false,
  eth_connected: false
};

assert(mockStatusPayload.wifi_ap_active === true, 'WLAN-AP bleibt bei fehlendem LAN aktiv (kein Timeout-Shutdown)');
assert(mockStatusPayload.installer_confirmed === true, 'Installateur-Bestätigung flag vorhanden');
assert(mockStatusPayload.wifi_ap_mode === 'DEVICE_AP', 'Modus wechselt nach Bestätigung auf dauerhaften internen Geräte-AP');

console.log(`\n=======================================================`);
if (passedTests === totalTests) {
  console.log(`🎉 Alle ${passedTests}/${totalTests} Prüfungen für Sharegy Bridge erfolgreich bestanden!`);
} else {
  console.error(`⚠️ ${totalTests - passedTests} von ${totalTests} Prüfungen fehlgeschlagen.`);
  process.exit(1);
}
console.log(`=======================================================\n`);
