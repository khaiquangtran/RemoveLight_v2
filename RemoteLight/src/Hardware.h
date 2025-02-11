#ifndef HARDWARE_H
#define HARDWARE_H

#include <map>
#include <Wire.h>
#include <Arduino.h>
#include "./Logging.h"
#include "./SignalType.h"
#include "./RTCDef.h"
#include "./Package.h"

typedef unsigned long UL;

enum class BUTTON_VALUE : uint8_t
{
  NONE = 0x00,
  BUTTON_1 = 0x01,
  BUTTON_2 = 0x02,
  BUTTON_3 = 0x03,
  BUTTON_4 = 0x04,
  BUTTON_5 = 0x05,
  BUTTON_6 = 0x06,
  BUTTON_UP = 0x47,
  BUTTON_DOWN = 0x4B,
  BUTTON_RIGHT = 0x4A,
  BUTTON_LEFT = 0x48,
  BUTTON_OK = 0x49,
  BUTTON_MENU = 0x4D,
  BUTTON_APP = 0x56,
  BUTTON_BACK = 0x4C
};
class Hardware
{
public:
  Hardware() {};
  virtual ~Hardware() = default;

  virtual void handleSignal(const SignaLType signal, Package *data = nullptr) = 0;

  virtual void listenning(BUTTON_VALUE value)
  {
    LOGI(".");
  };

  virtual void listenning()
  {
    LOGI(".");
  };

protected:
  uint8_t scanAddress(uint8_t addressInput);

  const uint8_t INVALID = 0x80;
};

#endif // HARDWARE_H