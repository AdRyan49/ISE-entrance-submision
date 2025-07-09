#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "eir59735987";
const char* password = "bbnjgf7PBM";

WebServer server(80);

String temp1 = "-", hum1 = "-", air1 = "-";
String temp2 = "-", hum2 = "-", air2 = "-";

void handleSensorData() {
  String id = server.arg("id");
  String temp = server.arg("temp");
  String hum = server.arg("hum");
  String air = server.arg("air");

  if (id == "room1") {
    temp1 = temp; hum1 = hum; air1 = air;
  } else if (id == "room2") {
    temp2 = temp; hum2 = hum; air2 = air;
  }

  Serial.printf("%lu,%s,%s,%s,%s\n", millis(), id.c_str(), temp.c_str(), hum.c_str(), air.c_str());
  server.send(200, "text/plain", "OK");
}

// Split the HTML into parts to avoid parsing issues
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Ryan Family Household Monitor</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', sans-serif; background: #f4f4f4; margin: 0; padding: 20px; }";
  html += "h1 { text-align: center; color: #2c3e50; margin-bottom: 20px; }";
  html += ".dashboard { display: flex; flex-wrap: wrap; justify-content: center; gap: 30px; }";
  html += ".room, .charts { background: #fff; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); padding: 20px; width: 300px; text-align: center; }";
  html += ".charts { width: 90%; max-width: 800px; }";
  html += ".value { font-size: 1.2em; margin: 10px 0; color: #555; }";
  html += ".alert { color: red; font-weight: bold; }";
  html += "button { margin-top: 10px; padding: 8px 14px; border: none; border-radius: 6px; background-color: #3498db; color: white; cursor: pointer; }";
  html += ".footer { margin-top: 30px; text-align: center; color: #aaa; font-size: 0.9em; }";
  html += "</style></head><body>";
  
  html += "<h1>Ryan Family Household Monitor</h1>";
  html += "<div class='dashboard'>";
  html += "<div class='room'>";
  html += "<h2>Room 1</h2>";
  html += "<div class='value'>Temp: <span id='t1'>--</span> °C</div>";
  html += "<div class='value'>Humidity: <span id='h1'>--</span> %</div>";
  html += "<div class='value'>Air: <span id='a1'>--</span> ppm</div>";
  html += "<div class='alert' id='alert1'></div>";
  html += "</div>";
  html += "<div class='room'>";
  html += "<h2>Room 2</h2>";
  html += "<div class='value'>Temp: <span id='t2'>--</span> °C</div>";
  html += "<div class='value'>Humidity: <span id='h2'>--</span> %</div>";
  html += "<div class='value'>Air: <span id='a2'>--</span> ppm</div>";
  html += "<div class='alert' id='alert2'></div>";
  html += "</div></div>";
  
  html += "<div class='charts'>";
  html += "<canvas id='tempChart'></canvas>";
  html += "<button onclick='downloadCSV()'>Export CSV</button>";
  html += "</div>";
  
  html += "<div class='footer'>&copy; 2025 Ryan Family | Powered by ESP32</div>";
  
  server.send(200, "text/html", html);
}

// Serve JavaScript as a separate file
void handleJS() {
  String js = "var labels = [];";
  js += "var temp1Data = [], temp2Data = [];";
  js += "var csvData = ['Time,Room,Temp,Humidity,Air'];";
  
  js += "var tempChart = new Chart(document.getElementById('tempChart'), {";
  js += "type: 'line',";
  js += "data: {";
  js += "labels: labels,";
  js += "datasets: [{";
  js += "label: 'Room 1 Temp (°C)',";
  js += "data: temp1Data,";
  js += "borderColor: 'rgb(255, 99, 132)',";
  js += "fill: false";
  js += "}, {";
  js += "label: 'Room 2 Temp (°C)',";
  js += "data: temp2Data,";
  js += "borderColor: 'rgb(54, 162, 235)',";
  js += "fill: false";
  js += "}]},";
  js += "options: {";
  js += "responsive: true,";
  js += "scales: { x: { display: true }, y: { beginAtZero: true } }";
  js += "}});";
  
  js += "function updateData() {";
  js += "fetch('/latest').then(function(res) { return res.json(); }).then(function(data) {";
  js += "var time = new Date().toLocaleTimeString();";
  js += "labels.push(time);";
  js += "if (labels.length > 20) labels.shift();";
  js += "temp1Data.push(parseFloat(data.temp1));";
  js += "if (temp1Data.length > 20) temp1Data.shift();";
  js += "temp2Data.push(parseFloat(data.temp2));";
  js += "if (temp2Data.length > 20) temp2Data.shift();";
  js += "tempChart.update();";
  js += "document.getElementById('t1').textContent = data.temp1;";
  js += "document.getElementById('h1').textContent = data.hum1;";
  js += "document.getElementById('a1').textContent = data.air1;";
  js += "document.getElementById('t2').textContent = data.temp2;";
  js += "document.getElementById('h2').textContent = data.hum2;";
  js += "document.getElementById('a2').textContent = data.air2;";
  js += "document.getElementById('alert1').textContent = parseFloat(data.air1) > 150 ? '⚠️ Poor Air Quality!' : '';";
  js += "document.getElementById('alert2').textContent = parseFloat(data.air2) > 150 ? '⚠️ Poor Air Quality!' : '';";
  js += "csvData.push(time + ',room1,' + data.temp1 + ',' + data.hum1 + ',' + data.air1);";
  js += "csvData.push(time + ',room2,' + data.temp2 + ',' + data.hum2 + ',' + data.air2);";
  js += "});};";
  
  js += "function downloadCSV() {";
  js += "var blob = new Blob([csvData.join('\\n')], { type: 'text/csv' });";
  js += "var url = URL.createObjectURL(blob);";
  js += "var a = document.createElement('a');";
  js += "a.href = url;";
  js += "a.download = 'sensor_data.csv';";
  js += "a.click();";
  js += "URL.revokeObjectURL(url);";
  js += "};";
  
  js += "setInterval(updateData, 5000);";
  js += "updateData();";
  
  server.send(200, "application/javascript", js);
}

// Modified root handler to include the script
void handleRootWithScript() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Ryan Family Household Monitor</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', sans-serif; background: #f4f4f4; margin: 0; padding: 20px; }";
  html += "h1 { text-align: center; color: #2c3e50; margin-bottom: 20px; }";
  html += ".dashboard { display: flex; flex-wrap: wrap; justify-content: center; gap: 30px; }";
  html += ".room, .charts { background: #fff; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); padding: 20px; width: 300px; text-align: center; }";
  html += ".charts { width: 90%; max-width: 800px; }";
  html += ".value { font-size: 1.2em; margin: 10px 0; color: #555; }";
  html += ".alert { color: red; font-weight: bold; }";
  html += "button { margin-top: 10px; padding: 8px 14px; border: none; border-radius: 6px; background-color: #3498db; color: white; cursor: pointer; }";
  html += ".footer { margin-top: 30px; text-align: center; color: #aaa; font-size: 0.9em; }";
  html += "</style></head><body>";
  
  html += "<h1>Ryan Family Household Monitor</h1>";
  html += "<div class='dashboard'>";
  html += "<div class='room'>";
  html += "<h2>Room 1</h2>";
  html += "<div class='value'>Temp: <span id='t1'>--</span> °C</div>";
  html += "<div class='value'>Humidity: <span id='h1'>--</span> %</div>";
  html += "<div class='value'>Air: <span id='a1'>--</span> ppm</div>";
  html += "<div class='alert' id='alert1'></div>";
  html += "</div>";
  html += "<div class='room'>";
  html += "<h2>Room 2</h2>";
  html += "<div class='value'>Temp: <span id='t2'>--</span> °C</div>";
  html += "<div class='value'>Humidity: <span id='h2'>--</span> %</div>";
  html += "<div class='value'>Air: <span id='a2'>--</span> ppm</div>";
  html += "<div class='alert' id='alert2'></div>";
  html += "</div></div>";
  
  html += "<div class='charts'>";
  html += "<canvas id='tempChart'></canvas>";
  html += "<button onclick='downloadCSV()'>Export CSV</button>";
  html += "</div>";
  
  html += "<div class='footer'>&copy; 2025 Ryan Family | Powered by ESP32</div>";
  html += "<script src='/app.js'></script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleLatest() {
  String json = "{";
  json += "\"temp1\":\"" + temp1 + "\", ";
  json += "\"hum1\":\"" + hum1 + "\", ";
  json += "\"air1\":\"" + air1 + "\", ";
  json += "\"temp2\":\"" + temp2 + "\", ";
  json += "\"hum2\":\"" + hum2 + "\", ";
  json += "\"air2\":\"" + air2 + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("Server IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("timestamp_ms,room,temp_c,humidity_percent,air_ppm");

  server.on("/submit", HTTP_GET, handleSensorData);
  server.on("/", HTTP_GET, handleRootWithScript);
  server.on("/app.js", HTTP_GET, handleJS);
  server.on("/latest", HTTP_GET, handleLatest);
  server.begin();
}

void loop() {
  server.handleClient();
}