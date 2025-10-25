#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define i2c_address 0x3c

#define SD_MOSI 10
#define SD_MISO 9
#define SD_SCLK 8
#define SD_CS   7

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int button1 = 0;
const int button2 = 1;
const int button3 = 2;
const int button4 = 3;

void setup() {
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);  

  Serial.begin(115200);

  delay(1000);

  Serial.println("\ninitializing SD card...");

  // Start SPI on specific pins
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

  // Initialize SD card
  if (!SD.begin(SD_CS, SPI)) {
    Serial.println("card mount failed");
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("no SD card attached");
  }

  Wire.begin();  // start I2C
  display.begin(i2c_address, true); // true = perform reset
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(20, 20);

  // display.fillRect(0, 0, 128, 64, SH110X_WHITE);
  
  display.display();
}

void loop() {
  if(digitalRead(button1) == LOW){
    Serial.println("button 1");
    display.clearDisplay();
    display.setCursor(20, 20);
    display.print("button 1");
    display.display();
    delay(200);
  }
  if(digitalRead(button2) == LOW){
    Serial.println("button 2");
    display.clearDisplay();
    display.setCursor(20, 20);
    display.print("button 2");
    display.display();
    delay(200);
  }
  if(digitalRead(button3) == LOW){
    Serial.println("button 3");
    display.clearDisplay();
    display.setCursor(20, 20);
    display.print("button 3");
    display.display();
    delay(200);
  }
  if(digitalRead(button4) == LOW){
    Serial.println("button 4");
    display.clearDisplay();
    display.setCursor(20, 20);
    display.print("button 4");
    display.display();
    delay(200);
  }
}
