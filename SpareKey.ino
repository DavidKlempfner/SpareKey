//This code is a modified version of https://cdn.instructables.com/ORIG/FFD/AL3D/IN3EI5D1/FFDAL3DIN3EI5D1.txt
//Pin Reference: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const String ssid = "iiNetC2DAD7";
const String wifiPassword = "aM9PrxcbtkS";
const String password = "87";

const int doorPin = 5;
const int buzzerPin = 4;
const int toneDuration = 1000;

const int lockedOutTimeInSeconds = 10;
const int maxBadPasswordCount = 3;
int badPasswordCount = 0;
long timeWhenUserCanTryNewPassword = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

WiFiServer server(301); //just pick any port number you like
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println(WiFi.localIP());

  timeClient.begin();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  pinMode(doorPin, OUTPUT);
  digitalWrite(doorPin, 0);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, 0);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}

void loop() {      
  client = server.available();
  if (!client) {
    return;
  }

  while (!client.available()) {
    delay(1);
  }

  String req = client.readStringUntil('\r');
  client.flush();

  Serial.println(req);
  long currentEpochTime = GetCurrentEpochTime();
  bool doesUserStillHaveToWait = currentEpochTime < timeWhenUserCanTryNewPassword;
  if (doesUserStillHaveToWait){
    long remainingLockOutTime = timeWhenUserCanTryNewPassword - currentEpochTime;
    GenerateResponse("Please wait " + String(remainingLockOutTime) + " seconds");
  }
  else if (req.indexOf("/" + password) != -1) { //Is password correct?
    GenerateResponse("Password is correct");
    OpenDoor();
    CorrectPasswordSound();    
    badPasswordCount = 0;
  }
  //Got a GET request and it wasn't the favicon.ico request, must have been a bad password:
  else if (req.indexOf("favicon.ico") == -1) {
    badPasswordCount++;
    if (badPasswordCount < maxBadPasswordCount) {
      GenerateResponse("Password is incorrect.");
    }
    else { //Disrupt brute-force attacks:      
      GenerateResponse("Password is incorrect. Please wait " + String(lockedOutTimeInSeconds) + " seconds.");
      timeWhenUserCanTryNewPassword = GetCurrentEpochTime() + lockedOutTimeInSeconds;
      badPasswordCount = 0;
      TooManyIncorrectPasswordsSound();
    }
  }
}

long GetCurrentEpochTime(){
  timeClient.update();
  return timeClient.getEpochTime();
}

void OpenDoor() {
  digitalWrite(LED_BUILTIN, 0);
  digitalWrite(doorPin, 1);
  delay(500);
  digitalWrite(LED_BUILTIN, 1);
  digitalWrite(doorPin, 0);
}

void GenerateResponse(String text) {
  Serial.println(text);
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += "<br><h1><b>" + text + "</b></h1>";
  s += "</html>\n";
  client.flush();
  client.print(s);
  delay(1);
}

void CorrectPasswordSound() {
  //Play 4 sounds, 1600, 1700, 1800, 1900Hz
  for(int i = 16; i < 20; i++){
    int frequency = i * 100;
    tone(buzzerPin, frequency);
    delay(toneDuration);
    noTone(buzzerPin);
  }
}

void TooManyIncorrectPasswordsSound() {  
  int frequency = 400;    
  tone(buzzerPin, frequency);
  delay(3 * toneDuration);
  noTone(buzzerPin);  
}
