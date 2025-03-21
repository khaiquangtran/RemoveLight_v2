#include "./Button.h"

Button::Button(std::shared_ptr<RemoteLight> rml) : mRML(rml)
{
  LOGI("Initialization Button!");
  addButton(pinButton_1, SignalType::PRESS_BTN_1_SIGNAL);
  addButton(pinButton_2, SignalType::PRESS_BTN_2_SIGNAL);
  addButton(pinButton_3, SignalType::PRESS_BTN_3_SIGNAL);
}

Button::~Button()
{
}

void Button::handleSignal(const SignalType signal, Package *data)
{
  LOGI("Signal handled for button %d", signal);
  switch (signal)
  {
  case SignalType::PRESS_BTN_1_SIGNAL:
  case SignalType::PRESS_BTN_2_SIGNAL:
  case SignalType::PRESS_BTN_3_SIGNAL:
  {
    mRML->handleSignal(signal);
    break;
  }
  default:
  {
    LOGW("Button is not supported yet.");
    break;
  }
  }
}

void Button::addButton(uint8_t pin, SignalType signal)
{
  mListButton[pin] = std::make_pair(std::make_pair(0U, std::make_pair(true, true)), signal);
  pinMode(pin, INPUT_PULLUP);
}

void Button::listenning()
{
  for (auto &button : mListButton)
  {
    uint8_t pin = button.first;
    auto &debounceTime = button.second.first.first;
    auto &buttonState = button.second.first.second;
    bool currentState = digitalRead(pin);
    if (currentState != buttonState.second)
    {
      debounceTime = millis();
    }
    if ((millis() - debounceTime) > DEPAY)
    {
      if (currentState != buttonState.first)
      {
        buttonState.first = currentState;
        if (buttonState.first == LOW)
        {
          LOGI("Button %d pressed", pin);
          mRML->handleSignal(button.second.second);
        }
        else
        {
          LOGI("Button %d released", pin);
        }
      }
    }
    buttonState.second = currentState;
  }
}