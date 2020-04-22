#include <ESP8266WiFi.h>

const char* ssid = "iiNetC2DAD7";
const char* password = "aM9PrxcbtkS";

WiFiServer server(301); //just pick any port number you like
WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(10);
Serial.println(WiFi.localIP());  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

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
  if (req.indexOf("/87") != -1) { //Is password correct?
    digitalWrite(LED_BUILTIN, 0);
    delay(500);
    digitalWrite(LED_BUILTIN, 1);      
    GenerateResponse("Password is correct");
  }
  else if(req.indexOf("favicon.ico") == -1) {
    GenerateResponse("Password is incorrect");
  }
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
