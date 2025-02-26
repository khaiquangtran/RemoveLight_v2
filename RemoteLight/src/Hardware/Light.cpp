#include "./Light.h"

Light::Light(RemoteLight *rml) : mRML(rml)
{
	LOGI("Initialization Light!");
	mListLight[Light1] = HIGH;
	mListLight[Light2] = HIGH;
	mListLight[Light3] = HIGH;
	mListLight[Light4] = HIGH;

	std::map<uint8_t, bool>::iterator it;
	for (it = mListLight.begin(); it != mListLight.end(); it++)
	{
		pinMode(it->first, OUTPUT);
		digitalWrite(it->first, it->second);
	}
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
	{
		controlLight(Light1);
		break;
	}
	case SignaLType::IR_BTN_2_SIGNAL:
	case SignaLType::PRESS_BTN_2_SIGNAL:
	{
		controlLight(Light2);
		break;
	}
	case SignaLType::IR_BTN_3_SIGNAL:
	case SignaLType::PRESS_BTN_3_SIGNAL:
	{
		controlLight(Light3);
		break;
	}
	case SignaLType::IR_BTN_4_SIGNAL:
	{
		controlLight(Light4);
		break;
	}
	case SignaLType::RTC_TURN_ON_LIGHT1:
	{
		controlLight(Light1, 1);
		break;
	}
	case SignaLType::RTC_TURN_ON_LIGHT2:
	{
		controlLight(Light2, 1);
		break;
	}
	case SignaLType::RTC_TURN_ON_LIGHT3:
	{
		controlLight(Light3, 1);
		break;
	}
	case SignaLType::RTC_TURN_ON_LIGHT4:
	{
		controlLight(Light4, 1);
		break;
	}
	case SignaLType::RTC_TURN_OFF_LIGHT1:
	{
		controlLight(Light1, 0);
		break;
	}
	case SignaLType::RTC_TURN_OFF_LIGHT2:
	{
		controlLight(Light2, 0);
		break;
	}
	case SignaLType::RTC_TURN_OFF_LIGHT3:
	{
		controlLight(Light3, 0);
		break;
	}
	case SignaLType::RTC_TURN_OFF_LIGHT4:
	{
		controlLight(Light4, 0);
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
		if(mListLight[light]  != HIGH)
		{
			mListLight[light] = LOW;
			digitalWrite(light, mListLight[light]);
		}
	}
	else if(state == 0)
	{
		if(mListLight[light]  != LOW)
		{
			mListLight[light] = HIGH;
			digitalWrite(light, mListLight[light]);
		}
	}
	else {
		/*Do nothing*/
	}
}