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
  void handleSignal(const SignalType& signal, const Package* data = nullptr);
  void init();

private:
  void addButton(uint8_t pin, SignalType signal);

private:
struct ButtonInfo
  {
    unsigned long debounceTime = 0;
    bool prevRaw = HIGH;     // raw not debounce
    bool stableState = HIGH; // state after debounce

    unsigned long pressStart = 0;
    bool longPressed = false;

    SignalType signal; // short-press signal
  };

  std::shared_ptr<RemoteLight> mRML;
  std::map<uint8_t, ButtonInfo> mListButton;

  const UL DEPAY = 50U;
};

#endif // BUTTON_H
