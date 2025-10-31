#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include <WebServer.h>
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

WiFiServer server(3333);
WebServer webServer(80);

const int button1 = 0;
const int button2 = 2;
const int button3 = 1;
const int button4 = 3;
const int speaker = 10;

void printMessage(String message, int x, int y, bool clear) {
  display.setCursor(x + screenPaddingLeft, y + screenPaddingTop);

  if (clear) {
    display.clearDisplay();
  }

  display.print(message);
  display.display();
}

void serveRoot() {
  webServer.send_P(200, "text/html", html);
  marioSound();
}

void marioSound() {
  tone(speaker, 659, 125); delay(130);
  tone(speaker, 784, 125); delay(130);
  tone(speaker, 1319, 125); delay(130);
  tone(speaker, 1047, 125); delay(130);
  tone(speaker, 1175, 125); delay(130);
  tone(speaker, 1568, 125); delay(130);
  noTone(speaker);
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
  server.begin();

  // start html web server
  webServer.on("/", serveRoot);
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

  if(digitalRead(button1) == LOW){
    Serial.println("button 1");
    printMessage("button 1", 0, 0, true);
  }
  if(digitalRead(button2) == LOW){
    Serial.println("button 2");
    printMessage("button 2", 0, 0, true);
  }
  if(digitalRead(button3) == LOW){
    Serial.println("button 3");
    printMessage("button 3", 0, 0, true);

  }
  if(digitalRead(button4) == LOW){
    Serial.println("button 4");
    printMessage("button 4", 0, 0, true);
  }
}
