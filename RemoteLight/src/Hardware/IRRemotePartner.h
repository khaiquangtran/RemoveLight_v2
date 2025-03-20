#ifndef IR_REMOTE_PARTNER_H
#define IR_REMOTE_PARTNER_H

#include "./Hardware.h"
#include "./../RemoteLight.h"

class RemoteLight;

class IRRemotePartner : public Hardware
{
public:
  IRRemotePartner(std::shared_ptr<RemoteLight> rml);
  ~IRRemotePartner();
  IRRemotePartner(const IRRemotePartner &) = delete;
  IRRemotePartner &operator=(const IRRemotePartner &) = delete;

  void listenning();

  void handleSignal(const SignalType signal, Package *data = nullptr);

private:
  std::shared_ptr<RemoteLight> mRML;
  const uint_fast8_t pinIR = 26U;
  const int EEPROM_SIZE = 4*12;

  std::vector<SignalType> mButtonSignal;
  std::map<int32_t, SignalType> mButtonSignalMap;
  uint8_t mFlagInstallButton;
  uint8_t mNumberButton;
  int32_t mValueButton;
};

#endif // IR_REMOTE_PARTNER_H