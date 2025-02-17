#ifndef LIGHT_H
#define LIGHT_H

#include "./Hardware.h"
#include "./../RemoteLight.h"

class RemoteLight;

class Light : public Hardware
{
public:
  Light(RemoteLight *rml);
  ~Light();
  Light(const Light &) = delete;
  Light &operator=(const Light &) = delete;

  void handleSignal(const SignaLType signal, Package *data = nullptr);

private:
  void controlLight(uint8_t light);

  RemoteLight *mRML;
  const uint8_t Light1 = 18U;
  const uint8_t Light2 = 19U;
  const uint8_t Light3 = 5U;
  const uint8_t Light4 = 4U;
  std::map<uint8_t, bool> mListLight;
};

#endif // LIGHT_H