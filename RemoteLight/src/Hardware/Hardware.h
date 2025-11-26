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
#include "./../Utils/SignalToString.h"
#include "./RTCDef.h"

#define NOT_CONNECT_DEVICE 1
#define NOT_CONNECT_DEVICE_LCD 0

typedef unsigned long UL;
class Hardware
{
public:
  Hardware() {};
  virtual ~Hardware() = default;

  virtual void handleSignal(const SignalType& signal,const Package* data = nullptr) = 0;

  virtual void listenning()
  {
    LOGI(".");
  };
  virtual void init()
  {
    LOGI(".");
  };

protected:
  uint8_t scanAddress(uint8_t addressInput);
  int32_t *parseCommandStringToArray(String str, int32_t &size);

  const uint8_t INVALID = 0x80;

  const uint8_t SD_CS     = 5U;
  const uint8_t SPI_MOSI  = 23U;
  const uint8_t SPI_MISO  = 19U;
  const uint8_t SPI_SCK   = 18U;

  const uint8_t SCL     = 22U;
  const uint8_t SDA     = 21U;

  const uint8_t pinButton_1 = 34U;
  const uint8_t pinButton_2 = 35U;
  // const uint8_t pinButton_3 = 32U;

  const uint8_t Light1 = 25U;
  const uint8_t Light2 = 26U;
  const uint8_t Light3 = 27U;
  const uint8_t Light4 = 14U;

  const uint8_t Led1 = 5U;
  const uint8_t Led2 = 18U;
  const uint8_t Led3 = 19U;

  const uint_fast8_t pinIR = 13U;

};

#endif // HARDWARE_H