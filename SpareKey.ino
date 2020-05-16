//Pin Reference: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//Tone library: https://github.com/lbernstone/Tone
#include <WiFi.h>
#include <Tone32.h>

const size_t input_buffer_length = 256;

const char* ssid = "iiNetC2DAD7";
const char* wifiPassword = "aM9PrxcbtkS";
const char* passwordToOpenDoor = "/87"; //password should begin with a slash

const int doorPin = 17;
const int buzzerPin = 16;
const int toneDuration = 700;
const int buzzerChannel = 0;

const int LED_BUILTIN = 2;

boolean haveClient = false;

WiFiServer server(301); //Pick any port number you like

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println(WiFi.localIP());
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

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
  WiFiClient client = server.available();

  if (client) {    
    haveClient = true;
  } 
  else {
    haveClient = false;
  }

  if(haveClient && client.available()){    
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
}

void OpenDoor() {
  digitalWrite(LED_BUILTIN, 1); //flash the onboard LED to help during testing.
  digitalWrite(doorPin, 1);
  delay(500);
  digitalWrite(LED_BUILTIN, 0);
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
  client.print("</b></h1>");
  client.print("<br><h1><b>Come on up to level 5 :)</b></h1>");  
  client.print("</html>\r\n");
  client.flush();
}

void CorrectPasswordSound() {
  //Play 1700, 1800, 1900Hz
  for(int i = 17; i < 20; i++){
    int frequency = i * 100;
    tone(buzzerPin, frequency, toneDuration, buzzerChannel);
    noTone(buzzerPin, buzzerChannel);
  }
}
