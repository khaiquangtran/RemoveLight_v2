#ifndef HARDWARE_H
#define HARDWARE_H

#include <map>
#include <Wire.h>
#include <Arduino.h>
#include "./Logging.h"
#include "./ParamDef.h"
#include "./RTCDef.h"
#include "./Package.h"

enum class BUTTON_VALUE : uint32_t
{
  BUTTON_1 = 0xFF807F,
  BUTTON_2 = 0xFF40BF,
  BUTTON_3 = 0xFFC03F,
  BUTTON_4 = 0xFF20DF,
  BUTTON_5 = 0xFFA05F,
  BUTTON_6 = 0xFF609F,
  BUTTON_UP = 0xFFE21D,
  BUTTON_DOWN = 0xFFD22D,
  BUTTON_RIGHT = 0xFF52AD,
  BUTTON_LEFT = 0xFF12ED,
  BUTTON_OK = 0xFF926D,
  BUTTON_MENU = 0xFFB24D,
  BUTTON_APP = 0xFF6A95,
  BUTTON_BACK = 0xFF32CD
};

typedef unsigned long UL;

class Hardware
{
public:
  Hardware() {};
  virtual ~Hardware() = default;

  virtual void handleSignal(const SignaLType signal, Package *data = nullptr) = 0;

  virtual void listenning(BUTTON_VALUE value) {};
  // virtual void listenButton() {};
  virtual void listenning() {};

protected:
  uint8_t scanAddress(uint8_t addressInput);

  const uint8_t INVALID = 0x80;
};

#endif // HARDWARE_H