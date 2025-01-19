#ifndef IR_REMOTE_PARTNER_H
#define IR_REMOTE_PARTNER_H

#include "./Hardware.h"
#include "./RemoteLight.h"

class RemoteLight;

class IRRemotePartner : public Hardware
{
public:
  IRRemotePartner(RemoteLight *rml);
  ~IRRemotePartner();
  IRRemotePartner(const IRRemotePartner &) = delete;
  IRRemotePartner &operator=(const IRRemotePartner &) = delete;

  void listenning(BUTTON_VALUE value);

  void handleSignal(const SignaLType signal, Package *data = nullptr);

private:
  RemoteLight *mRML;
  const uint_fast8_t pinIR = 26U;
};

#endif // IR_REMOTE_PARTNER_H
