//This code is a modified version of https://cdn.instructables.com/ORIG/FFD/AL3D/IN3EI5D1/FFDAL3DIN3EI5D1.txt
//Pin Reference: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
//#include <NTPClient.h>
//#include <string.h>

const size_t input_buffer_length = 256;

const char* ssid = "iiNetC2DAD7";
const char* wifiPassword = "aM9PrxcbtkS";
const char* passwordToOpenDoor = "/87"; //password should begin with a slash

const int doorPin = 5;
const int buzzerPin = 4;
const int toneDuration = 700;

int numOfRequests = 0;

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
  Serial.println("Server started.");
  Serial.println(WiFi.localIP());
}

void loop() {
//  ResetServer();
  client = server.available();
  if (!client) {    
    return;
  }

  while(!client.available()){
    delay(1);  
  }
    
//  String request = client.readStringUntil('\r');
  char request[input_buffer_length];
  client.readBytesUntil('\r', request, input_buffer_length);  
  client.flush();
  Serial.println(request);
  
//  if (request.indexOf(passwordToOpenDoor) != -1) { //Is password correct?
  if(strstr(request, passwordToOpenDoor)) {
    GenerateResponse("Password is correct");
    OpenDoor();
    CorrectPasswordSound();
  }
  //Got a GET request and it wasn't the favicon.ico request, must have been a bad password:
//  else if (request.indexOf("favicon.ico") == -1) {
  else if (!strstr(request, "favicon.ico")) {
    GenerateResponse("Password is incorrect.");
  }  
}

//void ResetServer() {
//  long currentTime = GetCurrentEpochTime();
//  long timeInterval = 300; //5mins
//  if (currentTime % timeInterval == 0) { //has timeInterval passed? If so, restart.
//    FlashLed();
//    server.stop();
//    delay(1000);
//    server.begin();
////    ESP.reset();
////    ESP.restart();
//    return;
//  }
//}

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

//void GenerateResponse(String text) {
//  Serial.println(text);
//  String s = "HTTP/1.1 200 OK\r\n";
//  s += "Content-Type: text/html\r\n\r\n";
//  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
//  s += "<br><h1><b>" + text + "</b></h1>";
//  s += "<br><h1><b>Bytes left on heap: " + String(ESP.getFreeHeap()) + "</b></h1>";
//  s += "<br><h1><b>Num of requests: " + String(++numOfRequests) + "</b></h1>";  
//  s += "</html>\n";
//  client.flush();
//  client.print(s);  
//}

void GenerateResponse(const char *text) {
  Serial.println(text);
  client.print(
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html\r\n"
      "\r\n"
      "<!DOCTYPE HTML>\r\n"
      "<html>\r\n"
      "<br><h1><b>"
  );
  client.print(text);
  client.print("</b></h1><br><h1><b>Num of requests: ");
  client.print(++numOfRequests);  
  client.print("</b></h1></html>\r\n");
  client.flush();  
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
