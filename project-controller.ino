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

void renderSDSpace() {
  if(SD.cardType() == CARD_NONE){
    printMessage("sd card not found", 0, 8, false);
  }else{
    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes  = SD.usedBytes();

    float totalGB = (float)totalBytes / (1024.0 * 1024.0 * 1024.0);
    float usedGB  = (float)usedBytes  / (1024.0 * 1024.0 * 1024.0);

    String message = "sd: ";
    message += String(usedGB, 1);  // one decimal place
    message += "gb/";
    message += String(totalGB, 1);
    message += "gb";

    printMessage(message, 0, 8, false);
    Serial.println(message);
  }
}

void renderMenuBase() {
    printMessage("menu:", 0, 0, true);
    display.fillRect(screenPaddingLeft, 10 + screenPaddingTop, 116, 1, SH110X_WHITE); // top border
    printMessage("connected clients", 10, 14, false);
    printMessage("network", 10, 22, false);
    printMessage("storage", 10, 30, false);
    printMessage("notifications", 10, 38, false);
    printMessage("reboot", 10, 46, false);
    display.display();
}

void renderMenuSelection(int oldSel, int newSel) {
    // erase old
    display.fillRect(3 + screenPaddingLeft, 17 + screenPaddingTop + (oldSel * 8), 3, 3, SH110X_BLACK);
    // draw new
    display.fillRect(3 + screenPaddingLeft, 17 + screenPaddingTop + (newSel * 8), 3, 3, SH110X_WHITE);
    display.display();
}

void renderClientMenu() {
  printMessage("client menu", 0, 0, true);
}
void renderNetworkMenu() {
  printMessage("network menu", 0, 0, true);
}
void renderStorageMenu() {
  printMessage("storage menu", 0, 0, true);
}
void renderNotificationsMenu() {
  printMessage("notification menu", 0, 0, true);
}

void checkButtons() {
  static bool subMenuOpen = false;
  static bool comboTriggered = false;
  static unsigned long comboStartTime = 0;
  static unsigned long lastNavTime = 0;
  static unsigned long menuOpenTime = 0;
  static const unsigned long navDelay = 150;
  static const unsigned long menuDebounceDelay = 500;

  static int menuSelection = 0;
  static int prevSelection = menuSelection;

  int buttonPins[4] = {button1, button2, button3, button4};

  // debounce and read buttons
  for (int i = 0; i < 4; i++) {
      bool reading = digitalRead(buttonPins[i]);
      if (reading != lastButtonState[i]) {
          lastButtonTime[i] = millis();
      }
      if ((millis() - lastButtonTime[i]) > debounceDelay) {
          buttonState[i] = reading;
      }
      lastButtonState[i] = reading;
  }

  // exit menu
  if (menuOpen && digitalRead(button2) == LOW) {
    menuOpen = false;
    printMessage(WiFi.softAPIP().toString() + String(":80"), 0, 0, true);
    renderSDSpace();
    return;
  }
  // exit sub menu
  if (subMenuOpen && digitalRead(button2) == LOW) {
    subMenuOpen = false;
    menuOpen = true;
    display.clearDisplay();
    renderMenuBase();
    renderMenuSelection(menuSelection, menuSelection);
    return;
  }

  // ignore all input right after menu opens
  if (menuOpen && (millis() - menuOpenTime < menuDebounceDelay)) {
    return;
  }

  // exit menu
  if (menuOpen && digitalRead(button2) == LOW) {
    menuOpen = false;
    printMessage(WiFi.softAPIP().toString() + String(":80"), 0, 0, true);
    renderSDSpace();
    return;
  }

  // menu navigation
  if(menuOpen && (millis() - lastNavTime > navDelay)) {
    if(buttonState[3] == LOW && menuSelection < 4) {  // down/right
      prevSelection = menuSelection;
      menuSelection++;
      renderMenuSelection(prevSelection, menuSelection);
      lastNavTime = millis();
    } else if (buttonState[0] == LOW && menuSelection > 0) {  // up/left
      prevSelection = menuSelection;
      menuSelection--;
      renderMenuSelection(prevSelection, menuSelection);
      lastNavTime = millis();
    }
  }

  if (menuOpen && digitalRead(button3) == LOW) {
    menuOpen = false;
    subMenuOpen = true;
    if (menuSelection == 0) {
      renderClientMenu();
    } else if (menuSelection == 1) {
      renderNetworkMenu();
    } else if (menuSelection == 2) {
      renderStorageMenu();
    } else if (menuSelection == 3) {
      renderNotificationsMenu();
    } else if (menuSelection == 4) {
      printMessage("restarting...", 0, 0, true);
      display.display();
      delay(500);
      ESP.restart();
    }
  }


  // combo detection
  bool comboHeld = (buttonState[0] == LOW && buttonState[3] == LOW && buttonState[1] == HIGH && buttonState[2] == HIGH);

  if (comboHeld) {
    if (comboStartTime == 0) comboStartTime = millis();

    if (!comboTriggered && (millis() - comboStartTime >= 200)) {
      comboTriggered = true;

      if (!menuOpen) {
        menuOpen = true;
        menuOpenTime = millis();  // record when it opened
        display.clearDisplay();
        renderMenuBase();
        renderMenuSelection(menuSelection, menuSelection);
      }

      comboStartTime = 0;
    }
  } else {
    comboStartTime = 0;
    comboTriggered = false;
  }
}

void serveRoot() {
  webServer.send_P(200, "text/html", html);
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
  printMessage(WiFi.softAPIP().toString() + String(":80"), 0, 0, false);
  Serial.println(WiFi.softAPIP());

  // start html web server
  webServer.on("/", serveRoot);
  webServer.on("/send-text", textReceived);
  webServer.begin();

  // initialize sd card
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("card mount failed");
  }else{
    renderSDSpace();
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("no SD card attached");
  }else{
    Serial.println("sd card found");
  }

  speakerTone();
}

void loop() {
  webServer.handleClient();
  checkButtons(); // non-blocking button debouncing
  updateAudio(); //non-blocking tones
}
