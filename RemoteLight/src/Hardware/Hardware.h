#ifndef HARDWARE_H
#define HARDWARE_H

#include <map>
#include <Wire.h>
#include <Arduino.h>
#include <vector>
#include <memory>
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

  const int SD_CS     = 5U;
  const int SPI_MOSI  = 23U;
  const int SPI_MISO  = 19U;
  const int SPI_SCK   = 18U;

  const int SCL     = 22U;
  const int SDA     = 21U;

  const uint8_t pinButton_1 = 12U;
  const uint8_t pinButton_2 = 14U;
  const uint8_t pinButton_3 = 27U;

};

#endif // HARDWARE_H