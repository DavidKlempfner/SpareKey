//This code is a modified version of https://cdn.instructables.com/ORIG/FFD/AL3D/IN3EI5D1/FFDAL3DIN3EI5D1.txt
//https://www.instructables.com/id/Control-ESP8266-Over-the-Internet-from-Anywhere/
//Pin Reference: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <ESP8266WiFi.h>
#include <millisDelay.h>

const size_t input_buffer_length = 256;

const char* ssid = "iiNetC2DAD7";
const char* wifiPassword = "aM9PrxcbtkS";
const char* passwordToOpenDoor = "/87"; //password should begin with a slash

const int doorPin = 5;
const int buzzerPin = 4;
const int toneDuration = 700;

millisDelay delayTime;

WiFiServer server(301); //Pick any port number you like
WiFiClient client;

void setup() {
  delayTime.start(600000); //10 mins
  
  Serial.begin(115200);
  delay(10);
  Serial.println(WiFi.localIP());
  
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

void(* resetFunc) (void) = 0;

void loop() {  
  if (delayTime.justFinished()) {
    Serial.println("10secs passed. Reseting...");
    resetFunc();
    delayTime.repeat();
  } 
  
  WiFiClient client = server.available();
  if (!client) {    
    return;
  }

  while(!client.available()){
    delay(1);  
  }
    
  char request[input_buffer_length];
  client.readBytesUntil('\r', request, input_buffer_length);  
  Serial.println(request);
  
  if(strstr(request, passwordToOpenDoor)) {
    GenerateResponse(client, "Password is correct.");
    OpenDoor();
    CorrectPasswordSound();
  }  
  else if (!strstr(request, "favicon.ico")) {
    //Got a GET request and it wasn't the favicon.ico request, must have been a bad password:
    GenerateResponse(client, "Password is incorrect.");
  }  
}

void OpenDoor() {
  digitalWrite(LED_BUILTIN, 0); //flash the onboard LED to help during testing.
  digitalWrite(doorPin, 1);
  delay(500);
  digitalWrite(LED_BUILTIN, 1);
  digitalWrite(doorPin, 0);
}

void GenerateResponse(WiFiClient& client, const char *text) {
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
  client.print("</b></h1><br><h1><b>Come on up to level 5 :)");  
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
