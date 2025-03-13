#ifndef HARDWARE_H
#define HARDWARE_H

#include <map>
#include <Wire.h>
#include <Arduino.h>
#include <vector>
#include "./../Utils/Logging.h"
#include "./../Utils/SignalType.h"
#include "./../Utils/Package.h"
#include "./RTCDef.h"

typedef unsigned long UL;
class Hardware
{
public:
  Hardware() {};
  virtual ~Hardware() = default;

  virtual void handleSignal(const SignalType signal, Package *data = nullptr) = 0;

  virtual void listenning()
  {
    LOGI(".");
  };

protected:
  uint8_t scanAddress(uint8_t addressInput);
  int *parseCommandStringToArray(String str, int &size);

  const uint8_t INVALID = 0x80;
};

#endif // HARDWARE_H