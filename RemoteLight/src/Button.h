#ifndef BUTTON_H
#define BUTTON_H

#include "./Hardware.h"
#include "./RemoteLight.h"

using stateButton = std::pair<bool, bool>;

class RemoteLight;

class Button : public Hardware
{
public:
  Button(RemoteLight *rml);
  ~Button();
  Button(const Button &) = delete;
  Button &operator=(const Button &) = delete;

  void listenning() override;

  void handleSignal(const SignaLType signal, Package *data = nullptr);

private:
  void addButton(uint8_t pin, SignaLType signal);

private:
  RemoteLight *mRML;
  std::map<uint8_t, std::pair<std::pair<UL, stateButton>, SignaLType>> mListButton;
  const UL DEPAY = 50U;
  const uint8_t pinButton_1 = 12U;
  const uint8_t pinButton_2 = 14U;
  const uint8_t pinButton_3 = 27U;
};

#endif // BUTTON_H
