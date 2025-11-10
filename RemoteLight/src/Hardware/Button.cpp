#include "./Button.h"

Button::Button(std::shared_ptr<RemoteLight> rml) : mRML(rml)
{
  LOGI("================== Button ==================");
}

void Button::init()
{
  addButton(pinButton_1, SignalType::PRESS_BTN_1_SIGNAL);
  addButton(pinButton_2, SignalType::PRESS_BTN_2_SIGNAL);
  // addButton(pinButton_3, SignalType::PRESS_BTN_3_SIGNAL);
}

Button::~Button()
{
}

void Button::handleSignal(const SignalType& signal, const Package* data)
{
  LOGI("Signal handled for button %d", signal);
  switch (signal)
  {
  case SignalType::PRESS_BTN_1_SIGNAL:
  case SignalType::PRESS_BTN_2_SIGNAL:
  // case SignalType::PRESS_BTN_3_SIGNAL:
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
  static unsigned long comboStartTime = 0;
  static bool comboActive = false;
  static bool comboHandled = false;

  bool btn1Pressed = (digitalRead(pinButton_1) == LOW);
  bool btn2Pressed = (digitalRead(pinButton_2) == LOW);
  // bool btn3Pressed = (digitalRead(pinButton_3) == LOW);

  // --- CHECK COMBO ---
  if (btn1Pressed && btn2Pressed)
  {
    if (!comboActive)
    {
      comboActive = true;
      comboStartTime = millis();
      comboHandled = false;
      LOGI("Combo BTN1+BTN2 started");
    }

    // Nếu giữ đủ 3 giây và chưa handle
    if (!comboHandled && (millis() - comboStartTime >= 3000))
    {
      comboHandled = true;
      LOGI("BTN1 + BTN2 held for 3s -> trigger combo!");
      mRML->handleSignal(SignalType::PRESS_BTN_1_2_COMBO_SIGNAL);
    }

    // Stop processing individual button while combo is in progress
    return;
  }
  else
  {
    // reset combo state if released
    comboActive = false;
    comboStartTime = 0;
    comboHandled = false;
  }

  // --- PROCESS SEPARATE BUTTON ONLY IF NO COMBO ---
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
          // Light::getInstance()->handleSignal(button.second.second);
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

