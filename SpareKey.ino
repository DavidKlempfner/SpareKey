//This code is a modified version of https://cdn.instructables.com/ORIG/FFD/AL3D/IN3EI5D1/FFDAL3DIN3EI5D1.txt
//Pin Reference: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
//#include <NTPClient.h>

const char* ssid = "iiNetC2DAD7";
const char* wifiPassword = "aM9PrxcbtkS";
const char* passwordToOpenDoor = "/87"; //password should begin with a slash

const int doorPin = 5;
const int buzzerPin = 4;
const int toneDuration = 1000;
//
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

WiFiServer server(301); //Pick any port number you like
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println(WiFi.localIP());

//  timeClient.begin();
  
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

  server.begin();
  Serial.println("Server started. Diagnostics info:");
  Serial.println(WiFi.localIP());   
}

void loop() {
//  long currentTime = GetCurrentEpochTime();
//  if (currentTime % 1800 == 0) { //has 30mins passed? If so, restart.
//    FlashLed();    
//      ESP.reset();
////    ESP.restart();
//    return;
//  }
    
  client = server.available();
  if (!client) {    
    return;
  }
  
  delay(1);
  if(client.available()) {    
    String request = client.readStringUntil('\r'); 
    client.flush();
  
    Serial.println(request);  
    if (request.indexOf(passwordToOpenDoor) != -1) { //Is password correct?    
      GenerateResponse("Password is correct");
      OpenDoor();
      CorrectPasswordSound();    
    }
    //Got a GET request and it wasn't the favicon.ico request, must have been a bad password:
    else if (request.indexOf("favicon.ico") == -1) {  
      GenerateResponse("Password is incorrect.");
    }
  }
}

//void FlashLed(){
//  digitalWrite(LED_BUILTIN, 0);  
//  delay(300);
//  digitalWrite(LED_BUILTIN, 1);
//  delay(300);
//  digitalWrite(LED_BUILTIN, 0);  
//  delay(300);
//  digitalWrite(LED_BUILTIN, 1);  
//  delay(300);
//  digitalWrite(LED_BUILTIN, 0);  
//  delay(300);
//  digitalWrite(LED_BUILTIN, 1);
//}

//long GetCurrentEpochTime(){
//  timeClient.update();
//  return timeClient.getEpochTime();
//}

void OpenDoor() {
  digitalWrite(LED_BUILTIN, 0); //flash the onboard LED to help during testing.
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
  //Play 1700, 1800, 1900Hz
  for(int i = 17; i < 20; i++){
    int frequency = i * 100;
    tone(buzzerPin, frequency);
    delay(toneDuration);
    noTone(buzzerPin);
  }
}
