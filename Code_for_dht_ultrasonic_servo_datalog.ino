#include <Servo.h>
#include <DHT.h>
#include <DHT_U.h>
#include <NewPing.h>
#include "SD.h"
#include <Wire.h>
#include "RTClib.h"

#define trigPinM  12
#define echoPinM  11
#define max_distance 200

#define echoPinB 4
#define trigPinB 5

#define echoPinL 6
#define trigPinL 7

#define echoPinR 9
#define trigPinR 8

#define LOG_INTERVAL  100
#define SYNC_INTERVAL 100
uint32_t syncTime = 0;

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

long durationL;
int distanceL;

long durationR;
int distanceR;

float hum;  //variable for storing the humidity level of the area
float temp; //variable for storing the temperature of the area

int pos = 0;  //rotating the servo from sasme position
int it = 10;

RTC_DS1307 RTC; // define the Real Time Clock object
const int chipSelect = 10;// for the data logging shield, we use digital pin 10 for the SD cs line
File myfile;// the logging file

NewPing sonar(trigPinM, echoPinM, max_distance);
Servo myservo;

void setup() {
  pinMode(trigPinB, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPinB, INPUT); // Sets the echoPin as an INPUT
  pinMode(trigPinL, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPinL, INPUT); // Sets the echoPin as an INPUT
  pinMode(trigPinR, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPinR, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
  myservo.attach(3);

  initSDcard();// initialize the SD card
  createFile();// create a new file
  initRTC(); // initialize the RTC
  myfile.println("millis,DistanceL,DistanceR,DistanceB,Humidity,Temprature");
}
void loop() {

  DateTime now;   //sets the current date and time
  int i = 0;
  int t = 0;
  int a = 0;
  digitalWrite(trigPinB, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPinB, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinB, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPinB, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance of front: ");
  Serial.print(distance);
  Serial.println(" cm");

  digitalWrite(trigPinL, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPinL, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinL, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  durationL = pulseIn(echoPinL, HIGH);
  // Calculating the distance
  distanceL = durationL * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distanceL);
  Serial.println(" cm");

  digitalWrite(trigPinR, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPinR, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinR, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  durationR = pulseIn(echoPinR, HIGH);
  // Calculating the distance
  distanceR = durationR * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distanceR);
  Serial.println(" cm");

  // code for DHT22

  hum = dht.readHumidity();
  temp = dht.readTemperature();


  // code for the servo motor starts here. It rotates with 1 degree per second to measure the distance in front of it.

  for (i = 0; i < 180; i ++)
  {
    unsigned int uS = sonar.ping(); //reads the distance in front of the sensor
    myservo.write(i); //rotates the servo by 1
    delay(20);
    for (t = 0; t < it; t++)
    {
      uS = sonar.ping();    //again pings infront of the ultrasonic sensor
      a = uS / US_ROUNDTRIP_CM + a;
      delay(30);
    }

    a = a / (it - 1);
    t = 0; // resets the positon of the servo
    a = 0;
  }

  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));// delay for the amount of time we want between readings
  uint32_t m = millis(); // log milliseconds since starting
  myfile.print(m);           // milliseconds since start
  myfile.print(", ");
  now = RTC.now();// fetch the time
  myfile.print(distanceL);
  myfile.print("; ");
  
  myfile.print(distanceR);
  myfile.print("; ");
  
  myfile.print(distance);
  myfile.print("; ");

  myfile.print(hum);
  myfile.print(", ");

  myfile.print(temp);
  myfile.println(", ");
  
  myfile.flush();
}

void initSDcard()
{
  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}

void createFile()
{
  //file name must be in 8.3 format (name length at most 8 characters, follwed by a '.' and then a three character extension.
  char filename[] = "MLOG00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = i / 10 + '0';
    filename[5] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      myfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  Serial.print("Logging to: ");
  Serial.println(filename);
}

void initRTC()
{
  Wire.begin();
  if (!RTC.begin()) {
    myfile.println("RTC failed");
  }
}
