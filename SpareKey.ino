//Pin Reference: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
//Tone library: https://github.com/lbernstone/Tone
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Tone32.h>

const size_t input_buffer_length = 256;

const char* ssid = "iiNetC2DAD7";
const char* wifiPassword = "M9PrxcbtkS";
const char* passwordToOpenDoor = "/87"; //password should begin with a slash

const int doorPin = 17;
const int buzzerPin = 16;
const int toneDuration = 700;
const int buzzerChannel = 0;

const int lockedOutTimeInSeconds = 15;
const int maxBadPasswordCount = 3;
int badPasswordCount = 0;
long timeWhenUserCanTryNewPassword = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

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

  timeClient.begin();
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

    long currentEpochTime = GetCurrentEpochTime();
    bool doesUserStillHaveToWait = currentEpochTime < timeWhenUserCanTryNewPassword;
    if (doesUserStillHaveToWait){
      long remainingLockOutTime = timeWhenUserCanTryNewPassword - currentEpochTime;     
      char remainingLockOutTimeStrBuffer[16] = {'\0'};
      itoa(remainingLockOutTime, remainingLockOutTimeStrBuffer, 10);
      char stillHavetoWaitMsg[input_buffer_length];
      strcat(stillHavetoWaitMsg, "Please wait ");
      strcat(stillHavetoWaitMsg, remainingLockOutTimeStrBuffer);
      strcat(stillHavetoWaitMsg, " seconds");      
      GenerateResponse(client, stillHavetoWaitMsg);
    }
    else if(strstr(request, passwordToOpenDoor)) {
      GenerateResponse(client, "Password is correct.");
      OpenDoor();
      CorrectPasswordSound();
      badPasswordCount = 0;
    }  
    else if (!strstr(request, "favicon.ico")) {
      //Got a GET request and it wasn't the favicon.ico request, must have been a bad password:
      badPasswordCount++;
      if (badPasswordCount < maxBadPasswordCount) {
          GenerateResponse(client, "Password is incorrect.");
      }
      else { //Disrupt brute-force attacks:
        char lockedOutTimeInSecondsStrBuffer[16] = {'\0'};
        itoa(lockedOutTimeInSeconds, lockedOutTimeInSecondsStrBuffer, 10);
        char badPasswordAndStillHavetoWaitMsg[input_buffer_length] = {'\0'};
        strcat(badPasswordAndStillHavetoWaitMsg, "Password is incorrect. Please wait ");
        strcat(badPasswordAndStillHavetoWaitMsg, lockedOutTimeInSecondsStrBuffer);
        strcat(badPasswordAndStillHavetoWaitMsg, " seconds");      
        GenerateResponse(client, badPasswordAndStillHavetoWaitMsg);        
        timeWhenUserCanTryNewPassword = GetCurrentEpochTime() + lockedOutTimeInSeconds;
        badPasswordCount = 0;
        TooManyIncorrectPasswordsSound();
      }
    }
  }
}

long GetCurrentEpochTime(){
  timeClient.update();
  return timeClient.getEpochTime();
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

void TooManyIncorrectPasswordsSound() {  
  int frequency = 400;    
  int badPasswordToneDuration = 3 * toneDuration;
  tone(buzzerPin, frequency, badPasswordToneDuration, buzzerChannel);  
  noTone(buzzerPin, buzzerChannel);  
}
