#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ================= WIFI =================
const char* ssid = "Aldi Rizkiansyah";
const char* password = "hasilnya24";

// ================= MQTT SHIFTR.IO =================
const char* mqtt_server = "uts-esp32-air.cloud.shiftr.io";
const int mqtt_port = 1883;

const char* mqtt_user = "uts-esp32-air";
const char* mqtt_password = "uts-esp32-air";

// ================= MOCKAPI =================
const char* apiUrl = "https://69a2b493be843d692bd20691.mockapi.io/api/v1/water-monitor";

WiFiClient espClient;
PubSubClient client(espClient);

// ================= PIN LED =================
const int led1 = 18;
const int led2 = 19;
const int led3 = 23;
const int led4 = 5;

// ================= PIN HC-SR04 =================
const int trigPin = 13;
const int echoPin = 12;

// ================= TOPIC MQTT =================
const char* topicControl    = "uts/led/control";
const char* topicStatus     = "uts/led/status";
const char* topicWaterLevel = "uts/water/level";
const char* topicWaterStatus= "uts/water/status";

// =================================================
// FUNGSI BACA SENSOR ULTRASONIK
// =================================================
float readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  float distance = duration * 0.034 / 2.0;
  return distance;
}

// =================================================
// CALLBACK MQTT
// =================================================
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("========== MQTT MASUK ==========");
  Serial.print("Topic   : ");
  Serial.println(topic);
  Serial.print("Pesan   : ");
  Serial.println(message);

  if (message == "LED1_ON") {
    digitalWrite(led1, HIGH);
  } else if (message == "LED1_OFF") {
    digitalWrite(led1, LOW);
  } else if (message == "LED2_ON") {
    digitalWrite(led2, HIGH);
  } else if (message == "LED2_OFF") {
    digitalWrite(led2, LOW);
  } else if (message == "LED3_ON") {
    digitalWrite(led3, HIGH);
  } else if (message == "LED3_OFF") {
    digitalWrite(led3, LOW);
  } else if (message == "LED4_ON") {
    digitalWrite(led4, HIGH);
  } else if (message == "LED4_OFF") {
    digitalWrite(led4, LOW);
  }

  String ledStatus =
    "L1:" + String(digitalRead(led1)) +
    ",L2:" + String(digitalRead(led2)) +
    ",L3:" + String(digitalRead(led3)) +
    ",L4:" + String(digitalRead(led4));

  client.publish(topicStatus, ledStatus.c_str());

  Serial.print("Status  : ");
  Serial.println(ledStatus);
  Serial.println("================================");
}

// =================================================
// WIFI
// =================================================
void setup_wifi() {
  Serial.println();
  Serial.println("Menghubungkan ke WiFi...");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Terhubung");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
}

// =================================================
// MQTT RECONNECT
// =================================================
void reconnect() {
  while (!client.connected()) {
    Serial.println("Menghubungkan ke MQTT...");

    String clientId = "ESP32_R32_";
    clientId += String(random(1000));

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("MQTT Terhubung");

      client.subscribe(topicControl);

      Serial.print("Subscribe : ");
      Serial.println(topicControl);

    } else {
      Serial.print("Gagal MQTT, rc=");
      Serial.println(client.state());

      delay(5000);
    }
  }
}

// =================================================
// SETUP
// =================================================
void setup() {
  Serial.begin(115200);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// =================================================
// LOOP
// =================================================
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  float distance = readDistance();
  String statusAir = "";

  // ================= STATUS AIR + LED OTOMATIS =================
  if (distance > 40) {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    statusAir = "KOSONG";
  }

  else if (distance > 25) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    statusAir = "RENDAH";
  }

  else if (distance > 13) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    statusAir = "SEDANG";
  }

  else if (distance > 5) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, LOW);
    statusAir = "TINGGI";
  }

  else {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
    statusAir = "PENUH";
  }

  // ================= MQTT PUBLISH =================
  String distanceText = String(distance, 2);

  client.publish(topicWaterLevel, distanceText.c_str());
  client.publish(topicWaterStatus, statusAir.c_str());

  String ledStatus =
    "L1:" + String(digitalRead(led1)) +
    ",L2:" + String(digitalRead(led2)) +
    ",L3:" + String(digitalRead(led3)) +
    ",L4:" + String(digitalRead(led4));

  client.publish(topicStatus, ledStatus.c_str());

  // ================= HTTP POST KE MOCKAPI =================
  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;

  http.begin(secureClient, apiUrl);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{";
  jsonData += "\"distance\":" + String(distance, 2) + ",";
  jsonData += "\"status\":\"" + statusAir + "\",";
  jsonData += "\"led1\":" + String(digitalRead(led1) ? "true" : "false") + ",";
  jsonData += "\"led2\":" + String(digitalRead(led2) ? "true" : "false") + ",";
  jsonData += "\"led3\":" + String(digitalRead(led3) ? "true" : "false") + ",";
  jsonData += "\"led4\":" + String(digitalRead(led4) ? "true" : "false");
  jsonData += "}";

  int httpResponseCode = http.POST(jsonData);

  Serial.println("========== HTTP POST ==========");
  Serial.print("JSON      : ");
  Serial.println(jsonData);

  Serial.print("Response  : ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();

    Serial.println("Body:");
    Serial.println(response);
  } else {
    Serial.print("Error:");
    Serial.println(http.errorToString(httpResponseCode));
  }

  Serial.println("================================");

  http.end();

  // ================= SERIAL MONITOR =================
  Serial.println("========== MONITORING ==========");
  Serial.print("Jarak Air : ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Status    : ");
  Serial.println(statusAir);

  Serial.print("LED1      : ");
  Serial.println(digitalRead(led1));

  Serial.print("LED2      : ");
  Serial.println(digitalRead(led2));

  Serial.print("LED3      : ");
  Serial.println(digitalRead(led3));

  Serial.print("LED4      : ");
  Serial.println(digitalRead(led4));

  Serial.println("================================");
  Serial.println();

  delay(3000);
}