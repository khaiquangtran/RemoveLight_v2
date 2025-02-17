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
	case IR_BTN_1_SIGNAL:
	case PRESS_BTN_1_SIGNAL:
	{
		controlLight(Light1);
		break;
	}
	case IR_BTN_2_SIGNAL:
	case PRESS_BTN_2_SIGNAL:
	{
		controlLight(Light2);
		break;
	}
	case IR_BTN_3_SIGNAL:
	case PRESS_BTN_3_SIGNAL:
	{
		controlLight(Light3);
		break;
	}
	case IR_BTN_4_SIGNAL:
	{
		controlLight(Light4);
		break;
	}
	default:
	{
		LOGW("Signal is not supported yet.");
		break;
	}
	}
}

void Light::controlLight(uint8_t light)
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