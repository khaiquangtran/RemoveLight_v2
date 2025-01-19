#ifndef LCD16x2_H
#define LCD16x2_H

#include <LiquidCrystal_I2C.h>
#include "./Hardware.h"
#include "./RemoteLight.h"

class RemoteLight;

class LCD16x2 : public Hardware
{
public:
  LCD16x2(RemoteLight *rml);
  ~LCD16x2();
  LCD16x2(const LCD16x2 &) = delete;
  LCD16x2 &operator=(const LCD16x2 &) = delete;

  void handleSignal(const SignaLType signal, Package *data = nullptr);

private:
  bool checkAddress();
  void displayTimeFromDS1307(struct TimeDS1307 data);
  void displayStartSetupMode();
  void displayEndSetupMode();
  void displayMenuMode(uint8_t light);
  void displaySelectedMenuMode(int *data);
  void displayStartConnectWifi();
  void displayConnectingWifi();
  void displayConnectWifiSuccess();
  void displayConnectWifiFailed();
  void displayStartConnectFirebase();
  void displayConnectFirebaseSuccess();
  void displayConnectNTPSuccess();
  void displayConnectFBFailed();
  void displayConnectNTPFailed();

  RemoteLight *mRML;
  const uint8_t LCD_ADDR = 0x27;
  struct TimeDS1307 mTimeAll;
  LiquidCrystal_I2C *mLCD;
  char *DAY[8];
  struct TimeDS1307 mReceiverTime;
  uint8_t mRetry;
  const uint8_t RETRY = 3U;
};

#endif // LCD16x2_H