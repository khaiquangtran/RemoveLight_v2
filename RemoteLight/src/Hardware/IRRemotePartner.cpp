#include "IRRemotePartner.h"
#include <IRremote.hpp>
#include <EEPROM.h>

IRRemotePartner::IRRemotePartner(std::shared_ptr<RemoteLight> rml) : mRML(rml), mFlagInstallButton(0U), mValueButton(0), mNumberButton(0)
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
    data = data | (EEPROM.read(i));
    mButtonSignalMap[data] = mButtonSignal[i/4];
    LOGD("data: %x, order: %d", data, i/4);
  }
}

IRRemotePartner::~IRRemotePartner()
{
}

void IRRemotePartner::handleSignal(const SignalType signal, Package *data)
{
  switch (signal)
  {
  case SignalType::IR_INSTALL_BUTTON: {
    mFlagInstallButton = 1;
    mNumberButton = 1;
    break;
  }
  case SignalType::IR_INSTALL_BUTTON_DONE: {
    if(mNumberButton <= 12)
    {
      LOGI("Update Button map");
      if(mButtonSignalMap.find(mValueButton) != mButtonSignalMap.end()) {
        mButtonSignalMap.erase(mValueButton);
      }
      mButtonSignalMap[mValueButton] = mButtonSignal[mNumberButton - 1];
      EEPROM.write(mNumberButton * 4 - 4, ((mValueButton & 0xFF000000) >> 24));
      EEPROM.write(mNumberButton * 4 - 3, ((mValueButton & 0x00FF0000) >> 16));
      EEPROM.write(mNumberButton * 4 - 2, ((mValueButton & 0x0000FF00) >> 8));
      EEPROM.write(mNumberButton * 4 - 1,  (mValueButton & 0x000000FF));
      EEPROM.commit();

      const int size = 1;
      int data[size] = {mNumberButton};
      Package *pack = new Package(data, size);
      mRML->handleSignal(SignalType::REMOTE_LIGHT_IRBUTTON_INSTALL, pack);
      delete pack;
      mNumberButton++;
    }
    else {
      mFlagInstallButton = 0;
      mNumberButton = 0;
      mRML->handleSignal(SignalType::IR_INSTALL_BUTTON_COMPLETE);
    }
    break;
  }
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
        if(mFlagInstallButton == 0) {
          if(mButtonSignalMap.find(valueIR) != mButtonSignalMap.end()) {
            mRML->handleSignal(mButtonSignalMap[valueIR]);
          }
          else {
            LOGW("Data not found!");
          }
        }
        else if(mFlagInstallButton == 1) {
          mValueButton = valueIR;
          const int size = 3;
          int data[size] = {0, 0, 0};
          data[0] = mNumberButton;
          data[1] = (mValueButton & 0xFFFF0000) >> 16;
          data[2] = (mValueButton & 0xFFFF);
          Package *pack = new Package(data, size);
          mRML->handleSignal(SignalType::REMOTE_LIGHT_IRBUTTON_INSTALL, pack);
          delete pack;
        }
        else {

        }
      }
    }
  }
}