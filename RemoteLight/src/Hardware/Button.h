#ifndef BUTTON_H
#define BUTTON_H

#include "./Hardware.h"
#include "./../RemoteLight.h"

using stateButton = std::pair<bool, bool>;

class RemoteLight;

class Button : public Hardware
{
public:
  Button(std::shared_ptr<RemoteLight> rml);
  ~Button();
  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;

  void listenning() override;

  void handleSignal(const SignalType signal, Package *data = nullptr);

private:
  void addButton(uint8_t pin, SignalType signal);

private:
  std::shared_ptr<RemoteLight> mRML;
  std::map<uint8_t, std::pair<std::pair<UL, stateButton>, SignalType>> mListButton;
  const UL DEPAY = 50U;

};

#endif // BUTTON_H
