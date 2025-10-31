#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <WebServer.h>
// custom headers
#include <html.h>

#define i2c_address 0x3c

#define SD_MOSI 6
#define SD_MISO 5
#define SD_SCLK 4
#define SD_CS   7

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "project_controller";
const char* password = "32sandwiches";
const int screenPaddingLeft = 4;
const int screenPaddingTop = 2;

WebServer webServer(80);

const int button1 = 0;
const int button2 = 2;
const int button3 = 1;
const int button4 = 3;
const int speaker = 10;
bool menuOpen = false;

#include <audio.h> // gotta define speaker pin first

const int debounceDelay = 50; // ms
unsigned long lastButtonTime[4] = {0,0,0,0};
bool buttonState[4] = {HIGH,HIGH,HIGH,HIGH};
bool lastButtonState[4] = {HIGH,HIGH,HIGH,HIGH};
bool buttonPressed[4] = {false,false,false,false};

void printMessage(String message, int x, int y, bool clear) {
  display.setCursor(x + screenPaddingLeft, y + screenPaddingTop);
  if (clear) {
    display.clearDisplay();
  }
  display.print(message);
  display.display();
}

void checkButtons() {
    static bool comboTriggered = false;
    static unsigned long comboStartTime = 0;

    int buttonPins[4] = {button1, button2, button3, button4};

    for(int i=0; i<4; i++){
        bool reading = digitalRead(buttonPins[i]);

        if(reading != lastButtonState[i]){
            lastButtonTime[i] = millis();
        }

        if((millis() - lastButtonTime[i]) > debounceDelay){
            buttonState[i] = reading;
        }

        lastButtonState[i] = reading;
    }

    // Check if combo is held
    bool comboHeld = (buttonState[0] == LOW && buttonState[3] == LOW &&
                      buttonState[1] == HIGH && buttonState[2] == HIGH );

    if(comboHeld){
        if(comboStartTime == 0){
            comboStartTime = millis(); // start timing
        }
        else if(!comboTriggered && (millis() - comboStartTime >= 200) && menuOpen == false){
            comboTriggered = true;
            menuOpen = true;
            printMessage("menu:", 0, 0, true);
            display.fillRect(screenPaddingLeft, 10 + screenPaddingTop, 116, 1, SH110X_WHITE); // top border
            display.fillRect(3 + screenPaddingLeft, 17 + screenPaddingTop, 3, 3, SH110X_WHITE);
            display.fillRect(4 + screenPaddingLeft, 26 + screenPaddingTop, 1, 1, SH110X_WHITE);
            display.fillRect(4 + screenPaddingLeft, 34 + screenPaddingTop, 1, 1, SH110X_WHITE);
            display.fillRect(4 + screenPaddingLeft, 42 + screenPaddingTop, 1, 1, SH110X_WHITE);
            printMessage("connected clients", 10, 14, false);
            printMessage("storage", 10, 22, false);
            printMessage("notifications", 10, 30, false);
            printMessage("reboot", 10, 38, false);
            display.display();
            comboStartTime = 0;
        }
         else if(!comboTriggered && (millis() - comboStartTime >= 200) && menuOpen == true){
            comboTriggered = true;
            menuOpen = false;
            printMessage(String("IP: ") + WiFi.softAPIP().toString(), 0, 0, true);
        }
    } else {
        // combo released, reset timer and trigger
        comboStartTime = 0;
        comboTriggered = false;
    }
}

void serveRoot() {
  webServer.send_P(200, "text/html", html);
  marioSound();
}

void textReceived() {
  if (webServer.method() == HTTP_POST && !menuOpen) {
    if (webServer.hasArg("plain")) {
      String data = webServer.arg("plain");
      printMessage(data, 0, 0, true);
      Serial.println("Received text:");
      Serial.println(data);
    } else {
      Serial.println("no data received");
    }
  }
  webServer.send(200, "text/plain", "OK");
}

void setup() {
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);  
  pinMode(speaker, OUTPUT);

  Serial.begin(115200);

  Wire.begin();  // start i2c
  display.begin(i2c_address, true); // true = perform reset
  display.clearDisplay();
  display.display();
  display.setTextSize(1); 
  display.setTextColor(SH110X_WHITE);
  display.setCursor(screenPaddingLeft, screenPaddingTop);
  
  display.display();

  // initialize wireless access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  printMessage(String("IP: ") + WiFi.softAPIP().toString(), 0, 0, false);
  Serial.println(WiFi.softAPIP());

  // start html web server
  webServer.on("/", serveRoot);
  webServer.on("/send-text", textReceived);
  webServer.begin();

  // initialize SD card
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("card mount failed");
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("no SD card attached");
  }else{
    Serial.println("sd card found");
  }

  marioSound();
}

void loop() {
  webServer.handleClient();
  checkButtons(); // non-blocking button debouncing
  updateMarioSound(); //non-blocking tones

  bool button1State = digitalRead(button1);
  bool button2State = digitalRead(button2);
  bool button3State = digitalRead(button3);
  bool button4State = digitalRead(button4);

  // if(digitalRead(button1) == LOW){
  //   Serial.println("button 1");
  //   printMessage("button 1", 0, 0, true);
  // }
  // if(digitalRead(button2) == LOW){
  //   Serial.println("button 2");
  //   printMessage("button 2", 0, 0, true);
  // }
  // if(digitalRead(button3) == LOW){
  //   Serial.println("button 3");
  //   printMessage("button 3", 0, 0, true);

  // }
  // if(digitalRead(button4) == LOW){
  //   Serial.println("button 4");
  //   printMessage("button 4", 0, 0, true);
  // }
}
