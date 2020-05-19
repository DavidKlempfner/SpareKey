//Pin Reference: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//Tone library: https://github.com/lbernstone/Tone
#include <WiFi.h>
#include <Tone32.h>
#include <ESP32_MailClient.h>
#include <HTTPClient.h>

const size_t input_buffer_length = 256;

const char* ssid = "iiNetC2DAD7";
const char* wifiPassword = "aM9PrxcbtkS";
const char* passwordToOpenDoor = "/87"; //password should begin with a slash

const char* emailSenderAccount = "notifieripchange1@gmail.com";
const char* emailSenderPassword = "Test1234!";
const char* emailRecipient = "dklempfner@gmail.com";
const char* smtpServer = "smtp.gmail.com";
const int smtpServerPort = 465;
const char* emailSubject = "Your public IP address has changed";

const int doorPin = 17;
const int buzzerPin = 16;
const int toneDuration = 700;
const int buzzerChannel = 0;

const int LED_BUILTIN = 2;

boolean haveClient = false;

boolean hasIPChanged = true;

// The Email Sending data object contains config and data to send
SMTPData smtpData;

// Callback function to get the Email sending status
void sendCallback(SendStatus info);

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
  if(hasIPChanged){
    getIPAddress();
    char newIPAddress[16] = "123.223.443.123";   
    sendIPChangedEmail(newIPAddress);
    hasIPChanged = false;
  }

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

void getIPAddress(){
  HTTPClient http;
  Serial.println("begin http");
//  http.begin("https://api.ipify.org"); //Specify the URL
  http.begin("http://jsonplaceholder.typicode.com/comments?id=10"); //Specify the URL  
  Serial.println("finish begin http, start get...");
  int httpCode = http.GET();                                        //Make the request
  Serial.println("finished get...");

  if (httpCode > 0) { //Check for the returning code
      //.toCharArray(buf, len)
      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
    }

  else {
    Serial.println("Error on HTTP request");
  }

  http.end(); //Free the resources
}

void sendCallback(SendStatus msg) {
  Serial.println(msg.info());
 
  if (msg.success()) {
    Serial.println("Email was sent successfully.");
  }
}

void sendIPChangedEmail(const char *newIPAddress){
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("ESP32", emailSenderAccount);
  smtpData.setPriority("High");
  smtpData.setSubject(emailSubject);
//  char newIPAddress[16] = "124.1.43.32";
  smtpData.setMessage(newIPAddress, false);
  smtpData.addRecipient(emailRecipient);
  smtpData.setSendCallback(sendCallback);
  if (!MailClient.sendMail(smtpData)){
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
  }

  //Clear all data from Email object to free memory
  smtpData.empty();
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
