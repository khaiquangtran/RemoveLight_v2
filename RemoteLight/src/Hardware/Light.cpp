#include "./Light.h"

Light::Light(std::shared_ptr<RemoteLight> rml) : mRML(rml)
{
	mListLight[LIGHT_1] = HIGH;
	mListLight[LIGHT_2] = HIGH;
	mListLight[LIGHT_3] = HIGH;
	mListLight[LIGHT_4] = HIGH;
	mListLight[LED_BTN_RST] = LOW;
	mListLight[LED_BTN_1] = LOW;
	mListLight[LED_BTN_2] = LOW;

	mSignalLightMap[SignalType::IR_BTN_1_SIGNAL] 						= LIGHT_1;
	mSignalLightMap[SignalType::BTN_PRESS_BTN_1_SIGNAL] 				= LIGHT_1;
	mSignalLightMap[SignalType::RTC_TURN_ON_LIGHT1] 					= LIGHT_1;
	mSignalLightMap[SignalType::RTC_TURN_OFF_LIGHT1] 					= LIGHT_1;
	mSignalLightMap[SignalType::IR_BTN_2_SIGNAL] 						= LIGHT_2;
	mSignalLightMap[SignalType::BTN_PRESS_BTN_2_SIGNAL] 				= LIGHT_2;
	mSignalLightMap[SignalType::RTC_TURN_ON_LIGHT2] 					= LIGHT_2;
	mSignalLightMap[SignalType::RTC_TURN_OFF_LIGHT2] 					= LIGHT_2;
	mSignalLightMap[SignalType::IR_BTN_3_SIGNAL] 						= LIGHT_3;
	mSignalLightMap[SignalType::RTC_TURN_ON_LIGHT3] 					= LIGHT_3;
	mSignalLightMap[SignalType::RTC_TURN_OFF_LIGHT3] 					= LIGHT_3;
	mSignalLightMap[SignalType::IR_BTN_4_SIGNAL] 						= LIGHT_4;
	mSignalLightMap[SignalType::RTC_TURN_ON_LIGHT4] 					= LIGHT_4;
	mSignalLightMap[SignalType::RTC_TURN_OFF_LIGHT4] 					= LIGHT_4;
	mSignalLightMap[SignalType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST] 	= LIGHT_1;
	mSignalLightMap[SignalType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST] 	= LIGHT_2;
	mSignalLightMap[SignalType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST] 	= LIGHT_3;
	mSignalLightMap[SignalType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST] 	= LIGHT_4;

	std::map<uint8_t, bool>::iterator it;
	#if NOT_CONNECT_DEVICE
		LOGW("LIGHT define skipped. NOT_CONNECT_DEVICE is defined");
	#else
		for (it = mListLight.begin(); it != mListLight.end(); it++)
		{
			pinMode(it->first, OUTPUT);
			digitalWrite(it->first, it->second);
		}
	#endif
	LOGI("================== Light ==================");
}

Light::~Light()
{
}

void Light::handleSignal(const SignalType& signal, const Package* data)
{
	switch (signal)
	{
	case SignalType::IR_BTN_1_SIGNAL:
	case SignalType::BTN_PRESS_BTN_1_SIGNAL:
	case SignalType::IR_BTN_2_SIGNAL:
	case SignalType::BTN_PRESS_BTN_2_SIGNAL:
	case SignalType::IR_BTN_3_SIGNAL:
	case SignalType::IR_BTN_4_SIGNAL:
	{
		controlLight(mSignalLightMap[signal]);
		break;
	}

	case SignalType::RTC_TURN_ON_LIGHT1:
	case SignalType::RTC_TURN_ON_LIGHT2:
	case SignalType::RTC_TURN_ON_LIGHT3:
	case SignalType::RTC_TURN_ON_LIGHT4:
	{
		controlLight(mSignalLightMap[signal], 1);
		break;
	}
	case SignalType::RTC_TURN_OFF_LIGHT1:
	case SignalType::RTC_TURN_OFF_LIGHT2:
	case SignalType::RTC_TURN_OFF_LIGHT3:
	case SignalType::RTC_TURN_OFF_LIGHT4:
	{
		controlLight(mSignalLightMap[signal], 0);
		break;
	}
	case SignalType::WEB_GET_STATUS_DATA_REQUEST:
	{
		sendLightStatusToWeb();
		break;
	}
	case SignalType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST:
	case SignalType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST:
	case SignalType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST:
	case SignalType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST:
	{
		if(data->getSize() == 2)
		{
			const int32_t* parseData  = data->getPackage();
			// LOGI("%d", parseData[1]);
			controlLight(mSignalLightMap[signal], parseData[1]);
			mRML->handleSignal(SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE);
		}
		break;
	}
	default:
	{
		LOGW("Signal is not supported yet.");
		break;
	}
	}
}

void Light::controlLight(uint8_t light, uint8_t state)
{
	if(state == 3)
	{
		mListLight[light] = (mListLight[light] == HIGH ? LOW : HIGH);
		#if NOT_CONNECT_DEVICE == 0
			digitalWrite(light, mListLight[light]);
		#endif
		if (mListLight[light])
		{
			LOGI("Turn OFF %d", light);
		}
		else
		{
			LOGI("Turn ON %d", light);
		}
	}
	else if(state == 1)
	{
		if(mListLight[light]  != LOW)
		{
			mListLight[light] = LOW;
			#if NOT_CONNECT_DEVICE == 0
				digitalWrite(light, mListLight[light]);
			#endif
		}
	}
	else if(state == 0)
	{
		if(mListLight[light]  != HIGH)
		{
			mListLight[light] = HIGH;
			#if NOT_CONNECT_DEVICE == 0
				digitalWrite(light, mListLight[light]);
			#endif
		}
	}
	else {
		/*Do nothing*/
	}
}

void Light::sendLightStatusToWeb()
{
	std::vector<int32_t> vecData = {
		static_cast<int32_t>(!(mListLight[LIGHT_1])),
		static_cast<int32_t>(!(mListLight[LIGHT_2])),
		static_cast<int32_t>(!(mListLight[LIGHT_3])),
		static_cast<int32_t>(!(mListLight[LIGHT_4]))
	};
	std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
	mRML->handleSignal(SignalType::WEB_GET_STATUS_DATA_RESPONSE, packData.get());
}