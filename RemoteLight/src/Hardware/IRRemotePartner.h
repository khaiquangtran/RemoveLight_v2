#ifndef IR_REMOTE_PARTNER_H
#define IR_REMOTE_PARTNER_H

#include "./Hardware.h"
#include "./../RemoteLight.h"

class RemoteLight;

class IRRemotePartner : public Hardware
{
public:
  IRRemotePartner(RemoteLight *rml);
  ~IRRemotePartner();
  IRRemotePartner(const IRRemotePartner &) = delete;
  IRRemotePartner &operator=(const IRRemotePartner &) = delete;

  void listenning();

  void handleSignal(const SignalType signal, Package *data = nullptr);

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

private:
  RemoteLight *mRML;
  const uint_fast8_t pinIR = 26U;
  const int EEPROM_SIZE = 4*12;

  std::vector<SignalType> mButtonSignal;
  std::map<int32_t, SignalType> mButtonSignalMap;
};

#endif // IR_REMOTE_PARTNER_H
