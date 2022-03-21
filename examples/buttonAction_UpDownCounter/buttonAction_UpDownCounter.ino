#include "FS.h"
#include "Free_Fonts.h" // Include the header file attached to this sketch

#include <TFT_eSPI.h>              // Hardware-specific library
#include <TFT_eButton.h>           // Hardware-specific library
#include <LittleFS.h>              // Don't forget to create Data Folder
#include <FT6236.h>                // https://github.com/DustinWatts/FT6236
#include <Wire.h>

#define I2C_Freq 100000
#define SDA 33
#define SCL 27
#define TOUCH_THRESHOLD 80
#define SCREEN_ROTATION 2

uint16_t desiredTemp = 120;  // How hot do we want the H2O?

#define DEFAULT_TEMP 120  // System reset H2O Temp
#define MAX_H2O_TEMP 180
#define MIN_H2O_TEMP 100
#define TEMP_INCREMENT 5  // Long press temp change

TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
FT6236 ts = FT6236();

// Initial LCD Backlight brightness
int ledBrightness = 200;

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

TFT_eButton btnUP = TFT_eButton(&tft);
TFT_eButton btnDOWN = TFT_eButton(&tft);

#define BUTTON_W 100
#define BUTTON_H 50

// Create an array of button instances to use in for() loops
// This is more useful where large numbers of buttons are employed
TFT_eButton* btn[] = {&btnUP , &btnDOWN};;
uint8_t buttonCount = sizeof(btn) / sizeof(btn[0]);

void btnCommonAction(uint8_t btnNum, uint8_t longPressTime, uint8_t longPressInc)
{
  if (btn[btnNum]->justPressed())  {
    btn[btnNum]->drawSmoothButton(true);
    btn[btnNum]->setPressTime(millis());
  }
  // if button pressed for more than 1 sec...
  if (millis() - btn[btnNum]->getPressTime() >= longPressTime) {
    Serial.println("Button pressed for 1 second.......");
    btn[btnNum]->setPressTime(millis());
    switch (btnNum)  {
      case 0: // UP Button
        if (desiredTemp < MAX_H2O_TEMP)
          desiredTemp += longPressInc;
        else
          desiredTemp = MAX_H2O_TEMP;
        break;
      case 1:  // DOWN Button
        if (desiredTemp > MIN_H2O_TEMP)
          desiredTemp -= longPressInc;
        else
          desiredTemp = MIN_H2O_TEMP;
        break;
      default:
        break;
    }
  }
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawNumber(desiredTemp, tft.width()/2, ((tft.height()/2)-180), 8);
}

void btnUP_pressAction(void)
{
  if (btnUP.justPressed()) {
    Serial.println("UP button just pressed");
    btnUP.drawSmoothButton(true);
  }
}

void btnUP_releaseAction(void)
{
  static uint32_t waitTime = 1000;
  if (btnUP.justReleased()) {
    Serial.println("Left button just released");
    btnUP.drawSmoothButton(false);
    btnUP.setReleaseTime(millis());
    //waitTime = 10000;
    if (desiredTemp < MAX_H2O_TEMP)
      desiredTemp++;
    else
      desiredTemp = MAX_H2O_TEMP;
  }
  else {
    if (millis() - btnUP.getReleaseTime() >= waitTime) {
      //waitTime = 1000;
      btnUP.setReleaseTime(millis());
      //btnUP.drawSmoothButton(!btnUP.getState());
    }
  }
}

void btnDOWN_pressAction(void)
{
  if (btnDOWN.justPressed()) {
    //btnDOWN.drawSmoothButton(!btnDOWN.getState(), 3, TFT_BLACK, btnDOWN.getState() ? "OFF" : "ON");
    btnDOWN.drawSmoothButton(true);
    //Serial.print("Button toggled: ");
    Serial.println("DOWN button just pressed");
    //if (btnDOWN.getState()) Serial.println("ON");
    //else  Serial.println("OFF");
    btnDOWN.setPressTime(millis());
  }

  // if button pressed for more than 1 sec...
  if (millis() - btnDOWN.getPressTime() >= 1000) {
    Serial.println("Down Button pressed for 1 second.......");
    btnDOWN.setPressTime(millis());
    if (desiredTemp > MIN_H2O_TEMP)
      desiredTemp -= TEMP_INCREMENT;
    else
      desiredTemp = MIN_H2O_TEMP;
  }
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawNumber(desiredTemp, tft.width()/2, ((tft.height()/2)-180), 8);
//  else Serial.println("DOWN button is being pressed");
}

void btnDOWN_releaseAction(void)
{
  static uint32_t waitTime = 1000;
  if (btnDOWN.justReleased()) {
    Serial.println("Left button just released");
    btnDOWN.drawSmoothButton(false);
    btnDOWN.setReleaseTime(millis());
    //waitTime = 10000;
    if (desiredTemp > MIN_H2O_TEMP)
      desiredTemp--;
    else
      desiredTemp = MIN_H2O_TEMP;
  }
  else {
    if (millis() - btnDOWN.getReleaseTime() >= waitTime) {
      //waitTime = 1000;
      btnDOWN.setReleaseTime(millis());
      //btnDOWN.drawSmoothButton(!btnDOWN.getState());
    }
  }
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawNumber(desiredTemp, tft.width()/2, ((tft.height()/2)-180), 8);
}

void initButtons() {
  uint16_t x = (tft.width() - BUTTON_W) / 2;
  uint16_t y = tft.height() / 2 - BUTTON_H - 10;
  btnUP.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, "UP", 1);
  //btnUP.setPressAction(btnUP_pressAction);
  //btnUP.setPressAction(btnCommonAction(0, 1000, 5));  // btn#, long press time, long press increment
  btnUP.setLongPressAction(btnCommonAction);  // btn#, long press time, long press increment
  btnUP.setReleaseAction(btnUP_releaseAction);
  btnUP.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing

  y = tft.height() / 2 + 10;
  btnDOWN.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_BLACK, TFT_GREEN, "DOWN", 1);
  //btnDOWN.setPressAction(btnDOWN_pressAction);
  btnDOWN.setLongPressAction(btnCommonAction);
  btnDOWN.setReleaseAction(btnDOWN_releaseAction);
  btnDOWN.drawSmoothButton(false, 3, TFT_BLACK); // 3 is outline width, TFT_BLACK is the surrounding background colour for anti-aliasing
}


void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF18);

  Wire.begin(SDA, SCL);  // Needed for CTP

  if (!ts.begin(TOUCH_THRESHOLD, SDA, SCL))
    Serial.println("[INFO]: Capacitive touch FAILED to start");
  else
    Serial.println("[INFO]: Capacitive touch started!");

  // Check for backlight pin if not connected to VCC
  #ifndef TFT_BL
    Serial.println("No TFT backlight pin defined");
  #else
    // Setup PWM channel, ledc is a LED Control Function
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, ledBrightness); // Start @ initial Brightness
    //pinMode(TFT_BL, OUTPUT);
    //digitalWrite(TFT_BL, HIGH);
  #endif

  // Calibrate the touch screen and retrieve the scaling factors
  //touch_calibrate();
  initButtons();
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawNumber(DEFAULT_TEMP, tft.width()/2, ((tft.height()/2)-180), 8);
  
}

void loop() {
  static uint32_t scanTime = millis();
  uint16_t t_x = 9999, t_y = 9999; // To store the touch coordinates
  bool pressed = false; // Make sure we not use last loop's touch

  // Scan buttons every 50ms at most
  if (millis() - scanTime >= 50) {
    // Pressed will be set true if there is a valid touch on the screen
    //bool pressed = tft.getTouch(&t_x, &t_y);
    if (ts.touched())
    {
      // Retrieve a point
      TS_Point p = ts.getPoint();

      switch (tft.getRotation())
      {
        case 0:
          t_x = tft.width() - p.x;
          t_y = p.y;
          break;
        case 1:
          t_x = p.y;
          t_y = tft.height() - p.x;
        break;
        case 2:
          t_x = p.x; 
          t_y = tft.height() - p.y;
        break;
        case 3: 
          t_x = tft.width() - p.y;
          t_y = p.x;
        break;
      }
      //Serial.print("Touch coordinate are "); Serial.print(t_x); Serial.print(", "); Serial.println(t_y);
      
      pressed = true;
    }
    
    scanTime = millis();
    for (uint8_t b = 0; b < buttonCount; b++) {
      if (pressed) {
        if (btn[b]->contains(t_x, t_y)) {
          btn[b]->press(true);
          btn[b]->longPressAction(b, 1000, 5);  // btn#, long press time, long press increment
        }
      }
      else {
        btn[b]->press(false);
        btn[b]->releaseAction();
      }
    }
  }

}

void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("Formating file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
