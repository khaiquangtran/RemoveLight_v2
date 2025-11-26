#ifndef LCD16x2_H
#define LCD16x2_H

#include <LiquidCrystal_I2C.h>
#include "./Hardware.h"
#include "./../RemoteLight.h"

class RemoteLight;

class LCD16x2 : public Hardware
{
public:
  LCD16x2(std::shared_ptr<RemoteLight> rml);
  ~LCD16x2();
  LCD16x2(const LCD16x2 &) = delete;
  LCD16x2 &operator=(const LCD16x2 &) = delete;

  void handleSignal(const SignalType& signal,const Package *data = nullptr);
  void init();

private:
  bool checkAddress();
  void displayTimeFromDS1307(const Package *data);
  void displayStartSetupMode();
  void displayEndSetupMode();
  void displayMenuMode(const uint8_t light);
  void displaySelectedMenuMode(const int32_t* data);
  void displayStartConnectWifi();
  void displayConnectingWifi();
  void displayConnectWifiSuccess();
  void displayConnectWifiFailed();
  void displayStartConnectFirebase();
  void displayConnectFirebaseSuccess();
  void displayStartConnectNTP();
  void displayConnectNTPSuccess();
  void displayConnectFBFailed();
  void displayConnectNTPFailed();
  void displayInstallButton(const Package* data);
  void displayStartProvisioning(const Package* data);
  void displayProvisioningFailed();
  void displayProvisioningSuccess();

  std::shared_ptr<RemoteLight>mRML;
  const uint8_t LCD_ADDR = 0x27;
  struct TimeDS1307 mTimeAll;
  LiquidCrystal_I2C *mLCD;
  char *DAY[8];
  struct TimeDS1307 mReceiverTime;
  uint8_t mRetry;
  const uint8_t RETRY = 3U;
  std::map<uint8_t, String> mButtonStringMap;
};

#endif // LCD16x2_H