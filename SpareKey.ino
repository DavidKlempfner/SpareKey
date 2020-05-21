//https://www.forward.com.au/pfod/ArduinoProgramming/TimingDelaysInArduino.html
//Pin Reference: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//https://techtutorialsx.com/2017/05/19/esp32-http-get-requests/
//Tone library: https://github.com/lbernstone/Tone
#include <WiFi.h>
#include <Tone32.h>
#include <ESP32_MailClient.h>
#include <HTTPClient.h>
#include <millisDelay.h>

const size_t input_buffer_length = 256;

const char* ssid = "iiNetC2DAD7";
const char* wifiPassword = "M9PrxcbtkS";
const char* passwordToOpenDoor = "/87"; //password should begin with a slash
const char* emailSenderAccount = "notifieripchange1@gmail.com";
const char* emailSenderPassword = "Test1234!";
const char* emailRecipient = "dklempfner@gmail.com";
const char* smtpServer = "smtp.gmail.com";
const char* emailSubject = "Your public IP address has changed";
const int smtpServerPort = 465;
const int doorPin = 17;
const int buzzerPin = 16;
const int toneDuration = 700;
const int buzzerChannel = 0;
const int LED_BUILTIN = 2;
const int lengthOfIPAddress = 16;
char oldIPAddress[lengthOfIPAddress] = "";
char newIPAddress[lengthOfIPAddress] = "";
const int lockOutTime = 60000; //180000ms = 3 mins
const int badPasswordCountLimit = 3;
int badPasswordCount = 0;
bool isLockedOut = false;

millisDelay lockedOutTimer;
millisDelay ipCheckTimer;
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

  ipCheckTimer.start(360000); //6 mins
  setIPAddress(oldIPAddress);
}

void loop() {   
  WiFiClient client = server.available();
  if(client){
    delay(50);
    if(client.available()){
      char request[input_buffer_length];
      client.readBytesUntil('\r', request, input_buffer_length);  
      Serial.println(request);

      if(isLockedOut){             
        if(!lockedOutTimer.justFinished()) {
          GenerateResponse(client, "You're locked out due to too many bad password attempts. Please try again later."); 
        }
        else {          
          isLockedOut = false;
          badPasswordCount = 0;
        }
      }
      
      if(!isLockedOut) {
        bool isPasswordCorrect = strstr(request, passwordToOpenDoor);
        if(isPasswordCorrect) {
          GenerateResponse(client, "Password is correct.");
          OpenDoor();
          CorrectPasswordSound();
        }  
        else if (!strstr(request, "favicon.ico")) {
          //Got a GET request and it wasn't the favicon.ico request, must have been a bad password:
          GenerateResponse(client, "Password is incorrect.");
          badPasswordCount++;
          if(badPasswordCount == badPasswordCountLimit){
            isLockedOut = true;
            BadPasswordCountLimitSound();
            lockedOutTimer.start(lockOutTime);
          }
        }
      }
    }  
  }
  
  if (ipCheckTimer.justFinished()) {
    if(hasIPAddressChanged()) {
      sendIPChangedEmail(newIPAddress);
    }      
    ipCheckTimer.repeat();
  }
}

bool hasIPAddressChanged(){  
  Serial.println("timer finished");
  setIPAddress(newIPAddress);
  Serial.println("OldIPAddress:");        
  Serial.println(oldIPAddress);
  Serial.println("NewIPAddress:");
  Serial.println(newIPAddress);
  bool hasIPAddressChanged = strcmp(oldIPAddress, newIPAddress) != 0;
  Serial.println("hasIPAddressChanged?");
  Serial.println(hasIPAddressChanged);
  if(hasIPAddressChanged){
    copy(newIPAddress, oldIPAddress, lengthOfIPAddress);    
  }   
  return hasIPAddressChanged;
}

void copy(char* src, char* dst, int len) {
    memcpy(dst, src, sizeof(src[0])*len);
}

void setIPAddress(char* ipAddressBuffer){
  HTTPClient http;
  Serial.println("begin http");  
  http.begin("http://bot.whatismyipaddress.com/"); //Only allowed to call once per 5 mins.
  Serial.println("Adding header...");
  http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36");
  Serial.println("Start get...");  
  int httpCode = http.GET();
  Serial.println("Finished get...");

  if (httpCode > 0) {
    int index = 0;
    WiFiClient& stream = http.getStream();    
    while (stream.available()) {
      ipAddressBuffer[index] = stream.read();
      index++;
    }
    
    Serial.println(httpCode);
    Serial.println(ipAddressBuffer);
  }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end();
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
  smtpData.setMessage(newIPAddress, false);
  smtpData.addRecipient(emailRecipient);
  smtpData.setSendCallback(sendCallback);
  if (!MailClient.sendMail(smtpData)){
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
  }  
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


void BadPasswordCountLimitSound() {  
  int frequency = 400;    
  tone(buzzerPin, frequency, 3 * toneDuration, buzzerChannel);
  noTone(buzzerPin, buzzerChannel);
}
