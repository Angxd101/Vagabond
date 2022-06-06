#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

//pins that are used to control the motor shield

#define DRIVE_MOTOR_POWER D2 
#define DRIVE_MOTOR_DIRECTION D4

#define STEER_MOTOR_POWER D1
#define STEER_MOTOR_DIRECTION D3

// drivePower sets how fast the car goes
// Can be set between 0 and 1023 (although car problaly wont move if values are too low)
int drivePower = 1023;

// driveDirection sets what direction the car drives
// If the car is moving backwards when you press the forward button, change this to LOW
uint8_t driveDirection = LOW;

// steeringPower sets how fast the car turns
// Can be set between 0 and 1023 (again, car probably won't steer if the value is too low)
int steeringPower = 1023;

// steerDirection sets what direction the car steers
// If the car is steering right when you press the left button, change this to LOW
uint8_t steerDirection = HIGH;

// ----------------
// Set your WiFi SSID and Password here
// ----------------
const char* ssid = "WiFi_Name";
const char* password = "WiFi_Password";

ESP8266WebServer server(80);

const char *webpage =
#include "motorPage.h"
  ;

void handleRoot() {

  server.send(200, "text/html", webpage);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {

  pinMode(DRIVE_MOTOR_POWER, OUTPUT);    
  pinMode(DRIVE_MOTOR_DIRECTION, OUTPUT);
  pinMode(STEER_MOTOR_POWER, OUTPUT);
  pinMode(STEER_MOTOR_DIRECTION, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("WifiCar")) {
    Serial.println("MDNS Responder Started");
  }

  server.on("/", handleRoot);

  server.on("/forward", []() {
    Serial.println("forward");
    analogWrite(DRIVE_MOTOR_POWER, drivePower);
    digitalWrite(DRIVE_MOTOR_DIRECTION, !driveDirection);
    server.send(200, "text/plain", "forward");
  });

  server.on("/driveStop", []() {
    Serial.println("driveStop");
    analogWrite(DRIVE_MOTOR_POWER, 0);
    server.send(200, "text/plain", "driveStop");
  });

  server.on("/back", []() {
    Serial.println("back");
    analogWrite(DRIVE_MOTOR_POWER, drivePower);
    digitalWrite(DRIVE_MOTOR_DIRECTION, driveDirection);
    server.send(200, "text/plain", "back");
  });

  server.on("/right", []() {
    Serial.println("right");
    analogWrite(STEER_MOTOR_POWER, steeringPower);
    digitalWrite(STEER_MOTOR_DIRECTION, steerDirection);
    server.send(200, "text/plain", "right");
  });

  server.on("/left", []() {
    Serial.println("left");
    analogWrite(STEER_MOTOR_POWER, steeringPower);
    digitalWrite(STEER_MOTOR_DIRECTION, !steerDirection);
    server.send(200, "text/plain", "left");
  });

  server.on("/steerStop", []() {
    Serial.println("steerStop");
    analogWrite(STEER_MOTOR_POWER, 0);
    server.send(200, "text/plain", "steerStop");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop(void) {
  server.handleClient();
}
