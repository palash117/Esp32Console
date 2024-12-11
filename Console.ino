#include <sstream>
#include <WiFi.h>
#include <consoledriver.h>
#include <iostream>
#include <sstream>
#include "AiEsp32RotaryEncoder.h"
#include <credentials.h>

// Display

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);



// Encoder

#define ROTARY_ENCODER_A_PIN 13
#define ROTARY_ENCODER_B_PIN 14
#define ROTARY_ENCODER_BUTTON_PIN 27
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

template<typename T>
std::string toString(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}
void rotary_onButtonClick() {
  static unsigned long lastTimePressed = 0;  // Soft debouncing
  if (millis() - lastTimePressed < 500) {
    return;
  }
  lastTimePressed = millis();
  Serial.print("button pressed ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}
bool longHold = false;
void rotary_loop(Console* console) {
  //dont print anything unless value changed
  if (rotaryEncoder.encoderChanged()) {
    Serial.print("Value: ");
    Serial.println(rotaryEncoder.readEncoder());
    printToDisplay(toString(rotaryEncoder.readEncoder()));
    if (rotaryEncoder.readEncoder() > 500) {
      console->handleRight();
    } else if (rotaryEncoder.readEncoder() < 500) {
      console->handleLeft();
    }
    rotaryEncoder.setEncoderValue(500);
  }
  if (rotaryEncoder.isEncoderButtonClicked() && !rotaryEncoder.isEncoderButtonDown()) {
    if (longHold) {
      longHold = false;
      return;
    }
    rotary_onButtonClick();
    printToDisplay("button clicked");
    console->handleOk();
    //  beep();
  }
  if (rotaryEncoder.isEncoderButtonDown()) {
  }
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void encoderSetup() {

  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  bool circleValues = false;
  rotaryEncoder.setBoundaries(0, 1000, circleValues);
  rotaryEncoder.setAcceleration(0);
  rotaryEncoder.setEncoderValue(500);
  Serial.print("encoderSetup completed");
}
void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  for (int i = 0; i < 10; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    } else {
      break;
    }
  }
  //console.isWifiConnected(WiFi.status()==WL_CONNECTED);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void printToDisplay(std::string str) {
  return;
  /*
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(str.c_str());
  display.display();
  */
}
void displaySetup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Console");
  display.println("version:");
  display.println(version.c_str());
  display.display();
}
/////////////////////////////////////////////////////////////////////////////////////////
/***************************     DRIVER     ********************************************/

/***************************     DRIVER OVER    ********************************************/
/////////////////////////////////////////////////////////////////////////////////////////
// u8g2, u8glib
Console* console;
void setup(void) {
  Serial.begin(115200);
  cout<<"starting setup \n";
  console = setupConsole();
  displaySetup();
  wifiSetup();
  encoderSetup();
  // beepSmall();
}
void printClear() {
  display.clearDisplay();
};
void printPre() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
};
// void beep(){
//     tone(BUZZZER_PIN, NOTE_C4 , 400);
// }
// void beepSmall(){
//       tone(BUZZZER_PIN, NOTE_C4 , 50);
// }
void printDisplayLine(DisplayLine line) {

  std::string isSelectedStart = ">";
  std::string isSelectedEnd = "";
  stringstream s("");
  // s<<"S";
  if (!line.selected) {
    isSelectedStart = "";
    isSelectedEnd = "";
  }
  s << isSelectedStart;
  for (int j = 0; j < line.words.size(); j++) {
    if (j != 0) {
      s << line.seperator;
    }
    for (int k = 0; k < line.words[j].marginLeft; k++) {
      s << " ";
    }
    s << line.words[j].str;
    for (int k = 0; k < line.words[j].marginRight; k++) {
      s << " ";
    }
  }

  if (line.hasFlag){
    s<<" ";
    if (line.flagValue){
      s<<"T";
    }else{
      s<<"F";
    }
  }  

  s << isSelectedEnd;
  s << "\n";

  if (line.bold) {
    printoutBold(s.str());
  } else {
    printout(s.str());
  }
  if (line.isTopBar){
    
  }
};

void printoutBold(std::string str) {

  display.setTextSize(2);
  printWithLen(str, 11);
}
void printout(std::string str) {

  display.setTextSize(1);
  printWithLen(str, 21);
}
void printWithLen(std::string str, int maxLen) {
  if (str.length() >= maxLen) {
    display.print(str.c_str());
    return;
  }
  int diff = maxLen - str.length();
  if (diff % 2 == 0) {
    for (int i = 0; i < diff / 2; i++) {
      display.print(" ");
    }
  } else {
    for (int i = 0; i < diff / 2; i++) {
      display.print(" ");
    }
  }
  display.print(str.c_str());
}
void setDisplayDarkMode(bool isDarkMode){
  display.invertDisplay(!isDarkMode);
}
void setDisplayRotation(bool isInverted){
  int rotation =0;
  if(isInverted){
    rotation =2;
  }
  display.setRotation(rotation);
}
void printFlush() {
  display.display();
};
void printDisplay() {
  printClear();
  printPre();
  DisplayPage str = console->display();
  int maxLines = 4;
  int len = str.lines.size();
  if (str.lines.size() > maxLines) {
    len = maxLines;
  }
  printDisplayLine(str.topbar);
  for (int i = 0; i < len; i++) {
    printDisplayLine(str.lines[i]);
  }
  setDisplayDarkMode(str.isDarkMode);
  setDisplayRotation(str.isInverted);

  printFlush();
}
void loop(void) {
   rotary_loop(console);
   printDisplay();
   console->tick();

}