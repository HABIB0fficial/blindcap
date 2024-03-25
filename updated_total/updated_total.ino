#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17


#define FRONT_TRIGGER_PIN 23 // GPIO 23 for front sensor trigger
#define FRONT_ECHO_PIN 22    // GPIO 22 for front sensor echo
#define LEFT_TRIGGER_PIN 21  // GPIO 21 for left sensor trigger
#define LEFT_ECHO_PIN 19     // GPIO 19 for left sensor echo
#define RIGHT_TRIGGER_PIN 18 // GPIO 18 for right sensor trigger
#define RIGHT_ECHO_PIN 5     // GPIO 5 for right sensor echo
#define BUZZER_PIN 12        // GPIO 12 for buzzer
#define VIBRATOR_PIN 13      // GPIO 13 for vibrator
#define THRESHOLD_DISTANCE 60
#define normal_distance 10 // 90 centimeters


TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

const char* ssid = "Mni";     // Wi-Fi name
const char* password = "Habib123"; // Wi-Fi password
const char* gmapApiKey = "AIzaSyCtK1urMQmUDEj28HHuvmyxyLXZcgH8018"; // Google Maps API key

WiFiServer server(80);

void setup() {


  Serial.begin(115200);
  delay(1000);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  connectToWiFi();
  server.begin();

   pinMode(FRONT_TRIGGER_PIN, OUTPUT);
  pinMode(FRONT_ECHO_PIN, INPUT);

  pinMode(LEFT_TRIGGER_PIN, OUTPUT);
  pinMode(LEFT_ECHO_PIN, INPUT);

  pinMode(RIGHT_TRIGGER_PIN, OUTPUT);
  pinMode(RIGHT_ECHO_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR_PIN, OUTPUT);
}

void loop() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      // Optional: You can process GPS data here if needed
    }
  }

  handleClient();

    long front_duration, front_distance, left_duration, left_distance, right_duration, right_distance;

  // Front sensor measurement
  front_distance = measureDistance(FRONT_TRIGGER_PIN, FRONT_ECHO_PIN, &front_duration);
  Serial.print("Front Distance: ");
  Serial.print(front_distance);
  Serial.println(" cm");

  // Left sensor measurement
  left_distance = measureDistance(LEFT_TRIGGER_PIN, LEFT_ECHO_PIN, &left_duration);
  Serial.print("Left Distance: ");
  Serial.print(left_distance);
  Serial.println(" cm");

  // Right sensor measurement
  right_distance = measureDistance(RIGHT_TRIGGER_PIN, RIGHT_ECHO_PIN, &right_duration);
  Serial.print("Right Distance: ");
  Serial.print(right_distance);
  Serial.println(" cm");

  // Condition for left sensor obstacle detection
  if (left_distance < THRESHOLD_DISTANCE & left_distance > normal_distance ) {
    activateVibrator(); // Example: Activate vibrator for left obstacle
    delay(1000); // Keep vibrator on for 1 second
    deactivateBuzzer();
    deactivateVibrator();
    return; // Exit loop to prevent further checks
  }

  // Condition for right sensor obstacle detection
  if (right_distance < THRESHOLD_DISTANCE & right_distance > normal_distance) {
    activateBuzzer(); // Example: Activate buzzer for right obstacle
    delay(1000); // Keep buzzer on for 1 second
    deactivateBuzzer();
    deactivateVibrator();
    return; // Exit loop to prevent further checks
  }

  // Condition for front sensor obstacle detection
  if (front_distance < THRESHOLD_DISTANCE & front_distance > normal_distance) {
    activateBuzzer(); // Example: Activate buzzer for front obstacle
    activateVibrator(); // Example: Activate vibrator for front obstacle
    delay(1000); // Keep both on for 1 second
    deactivateBuzzer();
    deactivateVibrator();
  } else {
    // No obstacles detected, turn off buzzer and vibrator
    deactivateBuzzer();
    deactivateVibrator();
  }

  delay(1000); // Delay between distance measurements
}

long measureDistance(int triggerPin, int echoPin, long *duration) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  
  *duration = pulseIn(echoPin, HIGH, 10000);
  
  return *duration * 0.034 / 2;
}

void activateBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
}

void deactivateBuzzer() {
  digitalWrite(BUZZER_PIN, LOW);
}

void activateVibrator() {
  digitalWrite(VIBRATOR_PIN, HIGH);
}

void deactivateVibrator() {
  digitalWrite(VIBRATOR_PIN, LOW);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Connection failed. Restarting ESP32...");
      ESP.restart();
    }
  }

  Serial.println();
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void handleClient() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected");
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n') {
          if (currentLine.length() == 0) {
            sendMapPage(client);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    delay(10);
    client.stop();
    Serial.println("Client disconnected");
  }
}

void sendMapPage(WiFiClient client) {
  String html = "";
  html += "HTTP/1.1 200 OK\r\n";
  html += "Content-Type: text/html\r\n";
  html += "Connection: close\r\n";
  html += "\r\n";
  html += "<!DOCTYPE html>\r\n";
  html += "<html>\r\n";
  html += "<head>\r\n";
  html += "<script src='https://maps.googleapis.com/maps/api/js?key=";
  html += gmapApiKey;
  html += "'></script>\r\n";
  html += "<style>\r\n";
  html += "body {\r\n";
  html += "  font-family: 'Arial', sans-serif;\r\n";
  html += "  background: linear-gradient(to bottom, #87CEEB, #f4f4f4);\r\n";
  html += "  margin: 0;\r\n";
  html += "}\r\n";
  html += "h1 {\r\n";
  html += "  text-align: center;\r\n";
  html += "  color: #333;\r\n";
  html += "}\r\n";
  html += "#map {\r\n";
  html += "  height: 400px;\r\n";
  html += "  width: 80%;\r\n";
  html += "  margin: 20px auto;\r\n";
  html += "  border-radius: 8px;\r\n";
  html += "  box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);\r\n";
  html += "}\r\n";
  html += "p {\r\n";
  html += "  color: #555;\r\n";
  html += "  text-align: center;\r\n";
  html += "}\r\n";
  html += "</style>\r\n";
  html += "</head>\r\n";
  html += "<body>\r\n";
  html += "<h1>Real-time GPS Tracker</h1>\r\n";
  html += "<div id='map'></div>\r\n";
  html += "<p id='latitude'>Latitude: " + String(gps.location.lat(), 6) + "</p>\r\n";
  html += "<p id='longitude'>Longitude: " + String(gps.location.lng(), 6) + "</p>\r\n";
  html += "<p>Altitude: " + String(gps.altitude.meters()) + " meters</p>\r\n";
  html += "<script>\r\n";
  html += "var map;\r\n";
  html += "var marker;\r\n";
  html += "function initMap() {\r\n";
  html += "  var mapOptions = {\r\n";
  html += "    zoom: 16,\r\n";
  html += "    center: {lat: " + String(gps.location.lat(), 6) + ", lng: " + String(gps.location.lng(), 6) + "}\r\n";
  html += "  };\r\n";
  html += "  map = new google.maps.Map(document.getElementById('map'), mapOptions);\r\n";
  html += "  marker = new google.maps.Marker({\r\n";
  html += "    position: {lat: " + String(gps.location.lat(), 6) + ", lng: " + String(gps.location.lng(), 6) + "},\r\n";
  html += "    map: map,\r\n";
  html += "    title: 'Current Location'\r\n";
  html += "  });\r\n";
  html += "}\r\n";
  html += "function moveMarker(latitude, longitude) {\r\n";
  html += "  var newLatLng = new google.maps.LatLng(latitude, longitude);\r\n";
  html += "  map.setCenter(newLatLng);\r\n";
  html += "  marker.setPosition(newLatLng);\r\n";
  html += "  document.getElementById('latitude').innerHTML = 'Latitude: ' + latitude;\r\n";
  html += "  document.getElementById('longitude').innerHTML = 'Longitude: ' + longitude;\r\n";
  html += "}\r\n";
  html += "initMap();\r\n";
  html += "setInterval(function() {\r\n";
  html += "  fetch('/location')\r\n";
  html += "  .then(response => response.json())\r\n";
  html += "  .then(data => {\r\n";
  html += "    moveMarker(data.latitude, data.longitude);\r\n";
  html += "  });\r\n";
  html += "}, 1000); // Update every 5 seconds\r\n";
  html += "</script>\r\n";
  html += "</body>\r\n";
  html += "</html>\r\n";

  client.print(html);
}






