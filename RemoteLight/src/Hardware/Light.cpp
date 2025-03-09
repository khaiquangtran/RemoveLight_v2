#include "./Light.h"

Light::Light(RemoteLight *rml) : mRML(rml)
{
	mListLight[Light1] = HIGH;
	mListLight[Light2] = HIGH;
	mListLight[Light3] = HIGH;
	mListLight[Light4] = HIGH;

	mSignalLightMap[SignaLType::IR_BTN_1_SIGNAL] 						= Light1;
	mSignalLightMap[SignaLType::PRESS_BTN_1_SIGNAL] 					= Light1;
	mSignalLightMap[SignaLType::RTC_TURN_ON_LIGHT1] 					= Light1;
	mSignalLightMap[SignaLType::RTC_TURN_OFF_LIGHT1] 					= Light1;
	mSignalLightMap[SignaLType::IR_BTN_2_SIGNAL] 						= Light2;
	mSignalLightMap[SignaLType::PRESS_BTN_2_SIGNAL] 					= Light2;
	mSignalLightMap[SignaLType::RTC_TURN_ON_LIGHT2] 					= Light2;
	mSignalLightMap[SignaLType::RTC_TURN_OFF_LIGHT2] 					= Light2;
	mSignalLightMap[SignaLType::IR_BTN_3_SIGNAL] 						= Light3;
	mSignalLightMap[SignaLType::PRESS_BTN_3_SIGNAL] 					= Light3;
	mSignalLightMap[SignaLType::RTC_TURN_ON_LIGHT3] 					= Light3;
	mSignalLightMap[SignaLType::RTC_TURN_OFF_LIGHT3] 					= Light3;
	mSignalLightMap[SignaLType::IR_BTN_4_SIGNAL] 						= Light4;
	mSignalLightMap[SignaLType::RTC_TURN_ON_LIGHT4] 					= Light4;
	mSignalLightMap[SignaLType::RTC_TURN_OFF_LIGHT4] 					= Light4;
	mSignalLightMap[SignaLType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST] 	= Light1;
	mSignalLightMap[SignaLType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST] 	= Light2;
	mSignalLightMap[SignaLType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST] 	= Light3;
	mSignalLightMap[SignaLType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST] 	= Light4;

	std::map<uint8_t, bool>::iterator it;
	for (it = mListLight.begin(); it != mListLight.end(); it++)
	{
		pinMode(it->first, OUTPUT);
		digitalWrite(it->first, it->second);
	}

	LOGI("Initialization Light!");
}

Light::~Light()
{
}

void Light::handleSignal(const SignaLType signal, Package *data)
{
	switch (signal)
	{
	case SignaLType::IR_BTN_1_SIGNAL:
	case SignaLType::PRESS_BTN_1_SIGNAL:
	case SignaLType::IR_BTN_2_SIGNAL:
	case SignaLType::PRESS_BTN_2_SIGNAL:
	case SignaLType::IR_BTN_3_SIGNAL:
	case SignaLType::PRESS_BTN_3_SIGNAL:
	case SignaLType::IR_BTN_4_SIGNAL:
	{
		controlLight(mSignalLightMap[signal]);
		break;
	}

	case SignaLType::RTC_TURN_ON_LIGHT1:
	case SignaLType::RTC_TURN_ON_LIGHT2:
	case SignaLType::RTC_TURN_ON_LIGHT3:
	case SignaLType::RTC_TURN_ON_LIGHT4:
	{
		controlLight(mSignalLightMap[signal], 1);
		break;
	}
	case SignaLType::RTC_TURN_OFF_LIGHT1:
	case SignaLType::RTC_TURN_OFF_LIGHT2:
	case SignaLType::RTC_TURN_OFF_LIGHT3:
	case SignaLType::RTC_TURN_OFF_LIGHT4:
	{
		controlLight(mSignalLightMap[signal], 0);
		break;
	}
	case SignaLType::WEB_GET_STATUS_DATA_REQUEST:
	{
		sendLightStatusToWeb();
		break;
	}
	case SignaLType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST:
	case SignaLType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST:
	case SignaLType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST:
	case SignaLType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST:
	{
		if(data->getSize() == 2)
		{
			int *parseData  = data->getPackage();
			// LOGI("%d", parseData[1]);
			controlLight(mSignalLightMap[signal], parseData[1]);
			mRML->handleSignal(SignaLType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE);
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
		digitalWrite(light, mListLight[light]);
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
			digitalWrite(light, mListLight[light]);
		}
	}
	else if(state == 0)
	{
		if(mListLight[light]  != HIGH)
		{
			mListLight[light] = HIGH;
			digitalWrite(light, mListLight[light]);
		}
	}
	else {
		/*Do nothing*/
	}
}

void Light::sendLightStatusToWeb()
{
	std::map<uint8_t, bool>::iterator it;
	const int size = 4;
	int data[size];
	data[0] = static_cast<int>(!(mListLight[Light1]));
	data[1] = static_cast<int>(!(mListLight[Light2]));
	data[2] = static_cast<int>(!(mListLight[Light3]));
	data[3] = static_cast<int>(!(mListLight[Light4]));
	Package package(data, size);
	mRML->handleSignal(SignaLType::WEB_GET_STATUS_DATA_RESPONSE, &package);
}