#include "IRRemotePartner.h"
#include "EEPROMPartner.h"
#include <IRremote.hpp>

IRRemotePartner::IRRemotePartner(std::shared_ptr<RemoteLight> rml) : mRML(rml), mFlagInstallButton(0U), mValueButton(0), mNumberButton(0)
{
  #if NOT_CONNECT_DEVICE
    LOGW("IrReceiver.begin skipped. NOT_CONNECT_DEVICE is defined");
  #else
    IrReceiver.begin(pinIR, ENABLE_LED_FEEDBACK);
  #endif
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
    SignalType::IR_BTN_BACK_SIGNAL,
    SignalType::IR_BTN_5_SIGNAL,
    SignalType::IR_BTN_6_SIGNAL,
    SignalType::IR_BTN_7_SIGNAL,
  };

  NUMBER_BUTTONS = mButtonSignal.size();

  LOGI(" =========== IRRemotePartner =========== ");
}

IRRemotePartner::~IRRemotePartner()
{

}

void IRRemotePartner::handleSignal(const SignalType& signal,const Package* data)
{
  switch (signal)
  {
  case SignalType::IR_INSTALL_BUTTON: {
    mFlagInstallButton = 1;
    mNumberButton = 1;
    break;
  }
  case SignalType::IR_INSTALL_BUTTON_DONE: {
    if(mNumberButton <= NUMBER_BUTTONS)
    {
      LOGI("Update Button map");
      if(mButtonSignalMap.find(mValueButton) != mButtonSignalMap.end()) {
        mButtonSignalMap.erase(mValueButton);
      }
      mButtonSignalMap[mValueButton] = mButtonSignal[mNumberButton - 1];

      std::vector<int32_t> vecData = {mNumberButton, mValueButton};
      std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
      mRML->handleSignal(SignalType::REMOTE_LIGHT_IRBUTTON_INSTALL, packData.get());
      mNumberButton++;
    }
    else {
      mFlagInstallButton = 0;
      mNumberButton = 0;
      // mRML->handleSignal(SignalType::IR_INSTALL_BUTTON_COMPLETE);
    }
    break;
  }
  case (SignalType::IR_ERRPROM_SEND_DATA): {
    parseDataFromEEPROM(data);
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
          std::vector<int32_t> vecData(3, 0);
          vecData.push_back(mNumberButton);
          vecData.push_back((mValueButton & 0xFFFF0000) >> 16);
          vecData.push_back((mValueButton & 0xFFFF));
          std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
          mRML->handleSignal(SignalType::REMOTE_LIGHT_IRBUTTON_INSTALL, packData.get());
        }
        else {

        }
      }
    }
  }
}

void IRRemotePartner::parseDataFromEEPROM(const Package* data) {
	if(data == nullptr) {
		LOGE("Data from EEPROM is null.");
		return;
	}
	else {
    const int32_t size = data->getSize();
    const int32_t* value = data->getPackage();
		if(size != NUMBER_BUTTONS) {
			LOGE("Data from EEPROM with length is not %.", NUMBER_BUTTONS);
			return;
		}
		else {
			for(int32_t i = 0; i < size; i++)
      {
        mButtonSignalMap[value[i]] = mButtonSignal[i];
        LOGD("data: %x, order: %d", value[i], i);
      }
		}
	}
}