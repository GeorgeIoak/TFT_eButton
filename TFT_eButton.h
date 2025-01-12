/***************************************************************************************
// The following button class has been ported over from the Adafruit_GFX library.
// A slightly different implementation in this TFT_eButton library allows:
//
// 1. The button labels to be in any font
// 2. Allow longer labels
// 3. Allow text datum to be set
// 4. Allow invert state to be read via getState(), true = inverted
// 5. Define pressed and released callbacks per button instances
//
// The library uses functions present in TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI
***************************************************************************************/
#ifndef _TFT_eButtonH_
#define _TFT_eButtonH_

//Standard support
#include <Arduino.h>

#include <TFT_eSPI.h>

typedef void (*pressCallback)(void);
typedef void (*longPressCallback)(uint8_t btnNum, uint32_t longPressTime, uint16_t longPressInc);
typedef void (*releaseCallback)(void);
// GI 220814 Added TFT_eWidget which duplicates TFT_eButton
static void dummyButtonAction(void) { }; // In case user calls it but it is not defined!


class TFT_eButton : public TFT_eSPI {

 public:

  TFT_eButton(TFT_eSPI *tft);

  uint8_t page;

  // "Classic" initButton() uses centre & size
  void     initButton(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize, uint8_t p);

  // New/alt initButton() uses upper-left corner & size
  void     initButtonUL(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize, uint8_t p);
  
  // Adjust text datum and x, y deltas
  void     setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = MC_DATUM);

  void     setPressAction(pressCallback action);
  void     setLongPressAction(longPressCallback action);
  void     setReleaseAction(releaseCallback action);

  void     setPressTime(uint32_t pressTime)    { _pressTime  = pressTime; }
  void     setReleaseTime(uint32_t releaseTime){ _releaseTime = releaseTime; }
  uint32_t getPressTime(void)  { return _pressTime; }
  uint32_t getReleaseTime(void){ return _releaseTime; }



  void     drawButton(bool inverted = false, String long_name = "");
  void     drawSmoothButton(bool inverted = false, int16_t outlinewidth = -1, uint32_t bgcolor = 0x00FFFFFF, String long_name = "");
  bool     contains(int16_t x, int16_t y);

  void     press(bool p);
  bool     isPressed();
  bool     justPressed();
  bool     justReleased();

  bool     getState(void) {return _inverted;}        // Get inverted state, true = inverted
  pressCallback   pressAction   = dummyButtonAction; // Press   action callback
  longPressCallback longPressAction = {}; // Long Press   action callback
  releaseCallback releaseAction = dummyButtonAction; // Release action callback

 private:
  TFT_eSPI *_gfx;
  int16_t  _x1, _y1;              // Coordinates of top-left corner of button
  int16_t  _xd, _yd;              // Button text datum offsets (wrt centre of button)
  uint16_t _w, _h;                // Width and height of button
  uint8_t  _textsize, _textdatum; // Text size multiplier and text datum for button
  uint16_t _outlinecolor, _fillcolor, _textcolor, _outlinewidth, _bgcolor;
  char     _label[10]; // Button text is 9 chars maximum unless long_name used
  uint32_t _pressTime, _releaseTime;
  bool  _inverted, currstate, laststate; // Button states
};

#endif