#ifndef LIGHT_H
#define LIGHT_H

#include "./Hardware.h"
#include "./../RemoteLight.h"

class RemoteLight;

class Light : public Hardware
{
public:
  Light(std::shared_ptr<RemoteLight> rml);
  ~Light();
  Light(const Light &) = delete;
  Light &operator=(const Light &) = delete;

  void handleSignal(const SignalType& signal, const Package* data = nullptr);

private:
  void controlLight(uint8_t light, uint8_t state = 3);
  void sendLightStatusToWeb();

  std::shared_ptr<RemoteLight> mRML;

  std::map<uint8_t, bool> mListLight;
  std::map<SignalType, uint8_t> mSignalLightMap;

};

#endif // LIGHT_H