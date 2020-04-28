//This code is a modified version of https://cdn.instructables.com/ORIG/FFD/AL3D/IN3EI5D1/FFDAL3DIN3EI5D1.txt
//Pin Reference: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#include <ESP8266WiFi.h>

const String ssid = "iiNetC2DAD7";
const String wifiPassword = "aM9PrxcbtkS";
const String password = "87";
const int doorPin = 5;
const int buzzerPin = 4;

int badPasswordCount = 0;
const int maxBadPasswordCount = 3;
const int waitTime = 10000;

WiFiServer server(301); //just pick any port number you like
WiFiClient client;

void setup() {
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
  if (req.indexOf("/" + password) != -1) { //Is password correct?
    GenerateResponse("Password is correct");
    OpenDoor();
    MakeSound();
  }
  else if (req.indexOf("favicon.ico") == -1) { //Ignore the favicon.ico Get request.
    badPasswordCount++;
    if (badPasswordCount < maxBadPasswordCount) {
      GenerateResponse("Password is incorrect.");
    }
    else { //Disrupt brute-force attacks:
      int waitTimeInSeconds = waitTime / 1000;
      GenerateResponse(String("Password is incorrect. Please wait ") + waitTimeInSeconds + " seconds.");
//todo use NTP to get current time, and don't allow any more attempts for next x seconds.
      badPasswordCount = 0;
    }
  }
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
  s += "<br><b>" + text + "</b>";
  s += "</html>\n";
  client.flush();
  client.print(s);
  delay(1);
}

void MakeSound() {    
  int toneDuration = 1000;
  tone(buzzerPin, 400);
  delay(toneDuration);
  noTone(buzzerPin);
  tone(buzzerPin, 800);
  delay(toneDuration);
  noTone(buzzerPin);
  tone(buzzerPin, 1200);
  delay(toneDuration);
  noTone(buzzerPin);
}
