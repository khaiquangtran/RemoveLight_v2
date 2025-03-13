#include "IRRemotePartner.h"
#include <IRremote.hpp>
#include <EEPROM.h>

IRRemotePartner::IRRemotePartner(RemoteLight *rml) : mRML(rml)
{
  LOGI("Initialization IRRemotePartner!");
  IrReceiver.begin(pinIR, ENABLE_LED_FEEDBACK);
  EEPROM.begin(EEPROM_SIZE);

  mButtonSignal = {
    SignalType::IR_BTN_1_SIGNAL,
    SignalType::IR_BTN_2_SIGNAL,
    SignalType::IR_BTN_3_SIGNAL,
    SignalType::IR_BTN_4_SIGNAL,
    SignalType::IR_BTN_UP_SIGNAL,
    SignalType::IR_BTN_DOWN_SIGNAL,
    SignalType::IR_BTN_RIGHT_SIGNAL,
    SignalType::IR_BTN_LEFT_SIGNAL,
    SignalType::IR_BTN_OK_SIGNAL,
    SignalType::IR_BTN_MENU_SIGNAL,
    SignalType::IR_BTN_APP_SIGNAL,
    SignalType::IR_BTN_BACK_SIGNAL
  };

  for(int i = 0; i < EEPROM_SIZE; i++)
  {
    int32_t data = 0;
    data = data | (EEPROM.read(i++) << 24U);
    data = data | (EEPROM.read(i++) << 16U);
    data = data | (EEPROM.read(i++) << 8U);
    data = data | (EEPROM.read(i++));
    mButtonSignalMap[data] = mButtonSignal[i/4];
  }
}

IRRemotePartner::~IRRemotePartner()
{
}

void IRRemotePartner::handleSignal(const SignalType signal, Package *data)
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
      int32_t valueIR = static_cast<int32_t>(IrReceiver.decodedIRData.decodedRawData);
      if (valueIR == 0U) {
        return;
      }
      else {
        LOGI("Data = %x", valueIR);
        if(mButtonSignalMap.find(valueIR) != mButtonSignalMap.end()) {
          mRML->handleSignal(mButtonSignalMap[valueIR]);
        }
        else {
          LOGW("Data not found!");
        }
      }
    }
  }
}