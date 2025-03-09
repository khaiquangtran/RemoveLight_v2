#include "IRRemotePartner.h"
#include <IRremote.hpp>

IRRemotePartner::IRRemotePartner(RemoteLight *rml) : mRML(rml)
{
  LOGI("Initialization IRRemotePartner!");
  IrReceiver.begin(pinIR, ENABLE_LED_FEEDBACK);
}

IRRemotePartner::~IRRemotePartner()
{
}

void IRRemotePartner::handleSignal(const SignaLType signal, Package *data)
{
  switch (signal)
  {
  case 1:
    break;
  default:
    break;
  }
}

void IRRemotePartner::listenning()
{
  if (IrReceiver.decode()) {
    IrReceiver.resume();
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      return;
    }
    else {
      BUTTON_VALUE valueIR = static_cast<BUTTON_VALUE>(IrReceiver.decodedIRData.command);
      if (valueIR == BUTTON_VALUE::NONE) {
        return;
      }
      else {
        LOGI("Data = %x", valueIR);
        switch (valueIR)
        {
        case (BUTTON_VALUE::BUTTON_1):
          LOGI("Button 1 pressed");
          mRML->handleSignal(SignaLType::IR_BTN_1_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_2):
          LOGI("Button 2 pressed");
          mRML->handleSignal(SignaLType::IR_BTN_2_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_3):
          LOGI("Button 3 pressed");
          mRML->handleSignal(SignaLType::IR_BTN_3_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_4):
          LOGI("Button 4 pressed");
          mRML->handleSignal(SignaLType::IR_BTN_4_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_5):
          LOGI("Button 5 pressed");
          break;
        case (BUTTON_VALUE::BUTTON_6):
          LOGI("Button 6 pressed");
          break;
        case (BUTTON_VALUE::BUTTON_UP):
          LOGI("Button Up pressed");
          mRML->handleSignal(SignaLType::IR_BTN_UP_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_DOWN):
          LOGI("Button Down pressed");
          mRML->handleSignal(SignaLType::IR_BTN_DOWN_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_RIGHT):
          LOGI("Button Right pressed");
          mRML->handleSignal(SignaLType::IR_BTN_RIGHT_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_LEFT):
          LOGI("Button Left pressed");
          mRML->handleSignal(SignaLType::IR_BTN_LEFT_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_OK):
          LOGI("Button Ok pressed");
          mRML->handleSignal(SignaLType::IR_BTN_OK_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_MENU):
          LOGI("Button Menu pressed");
          mRML->handleSignal(SignaLType::IR_BTN_MENU_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_APP):
          LOGI("Button App pressed");
          mRML->handleSignal(SignaLType::IR_BTN_APP_SIGNAL);
          break;
        case (BUTTON_VALUE::BUTTON_BACK):
          LOGI("Button Back pressed");
          mRML->handleSignal(SignaLType::IR_BTN_BACK_SIGNAL);
          break;
        default:
          LOGW("Button is not supported yet.");
          break;
        }
      }
    }
  }
}