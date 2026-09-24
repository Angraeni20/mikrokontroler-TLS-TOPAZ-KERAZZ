// =====================================================
// TOPAZ MONITORING SYSTEM
// Proyek: Topaz Monitoring System - Sistem Monitoring & Penyiraman Otomatis (Smart Irrigation & Environmental Telemetry) berbasis ESP32 dengan sensor DHT22, servo katup, dan Web UI monitoring realtime.
// =====================================================
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define DHTPIN    4
#define DHTTYPE   DHT22
#define SERVOPIN  18

const char* ssid     = "FM WIFI";
const char* password = "Mouse123";

DHT dht(DHTPIN, DHTTYPE);
Servo katupSiram;
WebServer server(80);

float thresholdSuhu       = 30.0;
float thresholdKelembaban = 50.0;

float suhuTerakhir       = NAN;
float kelembabanTerakhir = NAN;

unsigned long waktuBacaSensor = 0;

const unsigned long INTERVAL_SENSOR = 2000;

bool sedangMenyiram = false;

const unsigned long DURASI_SIRAM = 600;

unsigned long waktuMulaiSiram = 0;
const char HTML_CONTENT[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0" >
    <title>Topaz Monitoring System</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0; }
        body {
            font-family: Arial, sans-serif;
            background: #f4f7fb;
            color: #1f2937;
            padding: 20px; }
        .container {
            max-width: 800px;
            margin: auto; }
        .header {
            background: #2563eb;
            color: white;
            padding: 25px;
            border-radius: 15px;
            margin-bottom: 20px; }
        .header h1 {
            margin-bottom: 8px; }
        .header p {
            opacity: 0.9; }
        .card {
            background: white;
            padding: 20px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08); }
        .card h2 {
            margin-bottom: 15px; }
        .monitor {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 15px; }
        .sensor {
            background: #f8fafc;
            padding: 20px;
            border-radius: 12px;
            text-align: center; }
        .sensor-title {
            font-size: 14px;
            color: #64748b;
            margin-bottom: 8px; }
        .sensor-value {
            font-size: 32px;
            font-weight: bold; }
        .input-group {
            margin-bottom: 15px; }
        label {
            display: block;
            margin-bottom: 6px;
            font-weight: bold; }
        input {
            width: 100%;
            padding: 12px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            font-size: 16px; }
        button {
            width: 100%;
            padding: 13px;
            border: none;
            border-radius: 8px;
            background: #2563eb;
            color: white;
            font-size: 16px;
            cursor: pointer; }
        button:hover {
            background: #1d4ed8; }
        .status {
            margin-top: 15px;
            padding: 12px;
            border-radius: 8px;
            text-align: center;
            font-weight: bold;
            background: #f1f5f9; }
        .footer {
            text-align: center;
            color: #64748b;
            font-size: 13px; }
        @media (max-width: 600px) {
            .monitor {
                grid-template-columns: 1fr; } }
    </style>
</head>
<body>
<div class="container">
    <div class="header">
        <h1>
            Topaz Monitoring System </h1>
        <p>
            Smart Irrigation & Environmental Telemetry </p>
    </div>
    <div class="card">
        <h2>
            Monitoring Realtime </h2>
        <div class="monitor">
            <div class="sensor">
                <div class="sensor-title">
                    SUHU REALTIME </div>
                <div class="sensor-value">
                    <span id="suhu">
                        -- </span>
                    °C </div>
            </div>
            <div class="sensor">
                <div class="sensor-title">
                    KELEMBABAN </div>
                <div class="sensor-value">
                    <span id="kelembaban">
                        -- </span>
                    % </div>
            </div>
        </div>
        <div class="status" id="status">
            Mengecek kondisi... </div>
    </div>
    <div class="card">
        <h2>
            Topaz Control Thresholds </h2>
        <div class="input-group">
            <label for="suhuInput">
                Threshold Suhu Max (°C) </label>
            <input type="number" id="suhuInput" value="30" step="0.1" >
        </div>
        <div class="input-group">
            <label for="humInput">
                Threshold Kelembaban Min (%) </label>
            <input type="number" id="humInput" value="50" step="0.1" >
        </div>
        <button onclick="simpanParameter()">
            Simpan Parameter Topaz </button>
    </div>
    <div class="footer">
        Topaz Platform • Node Aktif </div>
</div>
<script>
    function updateData() {
        fetch('/data').then(response => {
                if (!response.ok) {
                    throw new Error('Gagal mengambil data'); }
                return response.json(); }).then(data => {
                document.getElementById('suhu').textContent = data.suhu.toFixed(1);
                document.getElementById('kelembaban').textContent = data.kelembaban.toFixed(1);
                const status = document.getElementById('status');
                if (data.sedangMenyiram) {
                    status.textContent = "💧 KATUP SEDANG MENYIRAM"; } else if (data.perluSiram) {
                    status.textContent = "⚠️ KONDISI PERLU DISIRAM"; } else {
                    status.textContent = "✓ KONDISI NORMAL"; } }).catch(error => {
                console.error('Gagal mengambil data:', error); }); }
    function simpanParameter() {
        const suhu = document.getElementById('suhuInput').value;
        const hum = document.getElementById('humInput').value;
        fetch(`/set?suhu=${suhu}&hum=${hum}`).then(response => {
            if (!response.ok) {
                throw new Error('Gagal menyimpan parameter'); }
            return response.text(); }).then(data => {
            alert('Parameter berhasil disimpan!');
            updateData(); }).catch(error => {
            alert('Gagal menyimpan parameter.');
            console.error(error); }); }
    updateData();
    setInterval(updateData, 1000);
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", HTML_CONTENT); }

void handleData() {
    if (isnan(suhuTerakhir) || isnan(kelembabanTerakhir)) {
        server.send(500, "application/json", "{\"error\":\"Data sensor belum tersedia\"}");
        return; }
    bool perluSiram = (suhuTerakhir > thresholdSuhu) || (kelembabanTerakhir < thresholdKelembaban);
    String json = "{";
    json += "\"suhu\":" + String(suhuTerakhir, 1) + ",";
    json += "\"kelembaban\":" + String(kelembabanTerakhir, 1) + ",";
    json += "\"perluSiram\":" + String(perluSiram ? "true" : "false") + ",";
    json += "\"sedangMenyiram\":" + String(sedangMenyiram ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json); }

void handleSet() {
    if (server.hasArg("suhu")) {
        float nilaiSuhu = server.arg("suhu").toFloat();
        if (nilaiSuhu >= -40 && nilaiSuhu <= 80) {
            thresholdSuhu = nilaiSuhu; } }
    if (server.hasArg("hum")) {
        float nilaiHum = server.arg("hum").toFloat();
        if (nilaiHum >= 0 && nilaiHum <= 100) {
            thresholdKelembaban = nilaiHum; } }
    Serial.println();
    Serial.println("=== THRESHOLD DIPERBARUI ===");
    Serial.print("Threshold Suhu       : ");
    Serial.print(thresholdSuhu, 1);
    Serial.println(" °C");
    Serial.print("Threshold Kelembaban : ");
    Serial.print(thresholdKelembaban, 1);
    Serial.println(" %");
    Serial.println("==============================");
    server.send(200, "text/plain", "OK"); }

void updateSensor() {
    unsigned long waktuSekarang = millis();
    if (waktuSekarang - waktuBacaSensor < INTERVAL_SENSOR) {
        return; }
    waktuBacaSensor = waktuSekarang;
    float suhu = dht.readTemperature();
    float kelembaban = dht.readHumidity();
    if (isnan(suhu) || isnan(kelembaban)) {
        Serial.println("ERROR: Gagal membaca DHT22");
        return; }
    suhuTerakhir = suhu;
    kelembabanTerakhir = kelembaban;
    Serial.println();
    Serial.println("========== Topaz ==========");
    Serial.print("Suhu          : ");
    Serial.print(suhu, 1);
    Serial.println(" °C");
    Serial.print("Kelembaban    : ");
    Serial.print(kelembaban, 1);
    Serial.println(" %");
    Serial.print("Threshold Suhu: ");
    Serial.print(thresholdSuhu, 1);
    Serial.println(" °C");
    Serial.print("Threshold Hum : ");
    Serial.print(thresholdKelembaban, 1);
    Serial.println(" %");
    bool perluSiram = (suhu > thresholdSuhu) || (kelembaban < thresholdKelembaban);
    if (perluSiram) {
        Serial.println("STATUS        : PERLU SIRAM");
        if (!sedangMenyiram) {
            sedangMenyiram = true;
            waktuMulaiSiram = millis();
            katupSiram.write(90);
            Serial.println("KATUP         : TERBUKA"); } } else {
        Serial.println("STATUS        : NORMAL");
        if (sedangMenyiram) {
            sedangMenyiram = false;
            katupSiram.write(0);
            Serial.println("KATUP         : DITUTUP"); } }
    Serial.println("============================"); }

void updateServo() {
    if (!sedangMenyiram) {
        return; }
    unsigned long waktuSekarang = millis();
    if (waktuSekarang - waktuMulaiSiram >= DURASI_SIRAM) {
        katupSiram.write(0);
        sedangMenyiram = false;
        Serial.println("KATUP         : DITUTUP (DURASI SELESAI)"); } }

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("========================================");
    Serial.println("       Topaz Monitoring System");
    Serial.println("========================================");
    dht.begin();
    Serial.println("DHT22 initialized.");
    katupSiram.attach(SERVOPIN);
    katupSiram.write(0);
    Serial.println("Servo initialized.");
    WiFi.begin(ssid, password);
    Serial.print("Menghubungkan ke Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("."); }
    Serial.println();
    Serial.println("Wi-Fi terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/set", handleSet);
    server.begin();
    Serial.println("Web Server aktif.");
    Serial.println("========================================");
    Serial.println("          Topaz Node Online");
    Serial.println("========================================");
    waktuBacaSensor = millis() - INTERVAL_SENSOR; }

void loop() {
    server.handleClient();
    updateSensor();
    updateServo(); }
