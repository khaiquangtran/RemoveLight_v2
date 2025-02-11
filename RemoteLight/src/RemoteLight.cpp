#include "./RemoteLight.h"

RemoteLight::RemoteLight()
{
	mSERIAL = std::make_shared<SerialPartner>(this);
	mRTC = std::make_shared<RTC>(this);
	mIR = std::make_shared<IRRemotePartner>(this);
	mLCD = std::make_shared<LCD16x2>(this);
	mBTN = std::make_shared<Button>(this);
	mLIGHT = std::make_shared<Light>(this);

	mTimerList["DisplayTimeAll"] = std::make_pair(0U, std::make_pair(DURATION_TIMER_1S, std::make_pair(0U, NUMBER_30S)));
	mTimerList["DisplaySetUpMode"] = std::make_pair(0U, std::make_pair(DURATION_TIMER_1S, std::make_pair(0U, 0U)));
	mTimerList["CheckConnectWifi"] = std::make_pair(0U, std::make_pair(DURATION_TIMER_3S, std::make_pair(0U, NUMBER_10S)));

	mControlMode = CONTROL_MODE::NONE;
	mNow = 0U;
	mElapsed = 0U;

	mFlagStatusWifi = -1;
	mFlagStatusFirebase = -1;
	mFlagStatusNTP = -1;
	mCounter = 0U;
	mStateConnect = STATE_CONNECT::NONE;

	LOGI("Initialization RemoteLight!");
}

RemoteLight::~RemoteLight()
{
}

void RemoteLight::init()
{
	mControlMode = CONTROL_MODE::CHECK_CONNECT_WIFI;
	mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_WIFI);
	mTimerList["CheckConnectWifi"].first = millis();
	mStateConnect = STATE_CONNECT::WIFI;
}

void RemoteLight::handleSignal(const SignaLType signal, Package *data)
{
	LOGD("Handle signal value: %d", signal);
	switch (signal)
	{
	case (SignaLType::PRESS_BTN_1_SIGNAL):
	case (SignaLType::PRESS_BTN_2_SIGNAL):
	case (SignaLType::PRESS_BTN_3_SIGNAL):
	case (SignaLType::IR_BTN_1_SIGNAL):
	case (SignaLType::IR_BTN_2_SIGNAL):
	case (SignaLType::IR_BTN_3_SIGNAL):
	case (SignaLType::IR_BTN_4_SIGNAL):
	{
		mLIGHT->handleSignal(signal);
		break;
	}
	case (SignaLType::LCD_MENU_MODE_BACk):
	case (SignaLType::LCD_MENU_MODE_OK):
	case (SignaLType::LCD_MOVE_LEFT_MENU_MODE):
	case (SignaLType::LCD_MOVE_RIGHT_MENU_MODE):
	case (SignaLType::LCD_DISPLAY_ALL_TIME):
	{
		mLCD->handleSignal(signal, data);
		break;
	}
	case (SignaLType::IR_BTN_APP_SIGNAL):
	{
		if (mControlMode != CONTROL_MODE::SETUP_MODE &&
			mControlMode != CONTROL_MODE::MENU_MODE)
		{
			mControlMode = CONTROL_MODE::SETUP_MODE;
			mLCD->handleSignal(signal);
			mTimerList["DisplaySetUpMode"].first = millis();
		}
		break;
	}
	case (SignaLType::IR_BTN_UP_SIGNAL):
	{
		if (mControlMode == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_INCREASE_VALUE);
		}
		else if (mControlMode == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_INCREASE_VALUE_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_DOWN_SIGNAL):
	{
		if (mControlMode == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_DECREASE_VALUE);
		}
		else if (mControlMode == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_DECREASE_VALUE_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_LEFT_SIGNAL):
	{
		if (mControlMode == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_SHIFT_LEFT_VALUE);
		}
		else if (mControlMode == CONTROL_MODE::MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_LEFT_MENU_MODE);
		}
		else if (mControlMode == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_LEFT_INTO_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_RIGHT_SIGNAL):
	{
		if (mControlMode == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_SHIFT_RIGHT_VALUE);
		}
		else if (mControlMode == CONTROL_MODE::MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_RIGHT_MENU_MODE);
		}
		else if (mControlMode == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_RIGHT_INTO_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_OK_SIGNAL):
	{
		if (mControlMode == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_SETUP_MODE_OK);
		}
		else if (mControlMode == CONTROL_MODE::MENU_MODE)
		{
			mControlMode = CONTROL_MODE::INTO_MENU_MODE;
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mRTC->handleSignal(SignaLType::RTC_MENU_MODE_OK);
		}
		break;
	}
	case (SignaLType::IR_BTN_MENU_SIGNAL):
	{
		if (mControlMode != CONTROL_MODE::SETUP_MODE &&
			mControlMode != CONTROL_MODE::MENU_MODE)
		{
			mControlMode = CONTROL_MODE::MENU_MODE;
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mLCD->handleSignal(SignaLType::LCD_TURN_ON_LIGHT);
			mLCD->handleSignal(SignaLType::IR_BTN_MENU_SIGNAL);
		}
		break;
	}
	case (SignaLType::IR_BTN_BACK_SIGNAL):
	{
		if (mControlMode == CONTROL_MODE::INTO_MENU_MODE)
		{
			mControlMode = CONTROL_MODE::MENU_MODE;
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mRTC->handleSignal(SignaLType::RTC_BACK_MENU_MODE);
		}
		else if (mControlMode == CONTROL_MODE::MENU_MODE)
		{
			mControlMode = CONTROL_MODE::DISPLAY_ALL;
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mTimerList["DisplayTimeAll"].first = millis();
		}
		break;
	}
	case (SignaLType::REMOTE_LIGHT_END_SETUP_MODE):
	{
		mControlMode = CONTROL_MODE::END_SETUP_MODE;
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignaLType::LCD_DISPLAY_END_SETUP_MODE);
		mTimerList["DisplaySetUpMode"].first = millis();
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_WIFI_SUCCESS):
	{
		mFlagStatusWifi = 1;
		mLCD->handleSignal(SignaLType::LCD_CONNECT_WIFI_SUCCESS);
		delay(1000);
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_FIREBASE);
		mStateConnect = STATE_CONNECT::FIREBASE;
		mTimerList["CheckConnectWifi"].second.second.first = 0U;
		mTimerList["CheckConnectWifi"].first = millis();
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_WIFI_FAILED):
	{
		LOGW("Set mFlagStatusWifi = 0");
		mFlagStatusWifi = 0;
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_SUCCESS):
	{
		mFlagStatusFirebase = 1;
		mLCD->handleSignal(SignaLType::LCD_CONNECT_FIREBASE_SUCCESS);
		delay(1000);
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_NTP);
		mStateConnect = STATE_CONNECT::NTP;
		mTimerList["CheckConnectWifi"].second.second.first = 0U;
		mTimerList["CheckConnectWifi"].first = millis();
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_FAILED):
	{
		LOGW("Set mFlagStatusFirebase = 0");
		mFlagStatusFirebase = 0;
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_NTP_SUCCESS):
	{
		mFlagStatusNTP = 1;
		mLCD->handleSignal(SignaLType::LCD_CONNECT_NTP_SUCCESS);
		delay(1000);
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mTimerList["CheckConnectWifi"].second.second.first = 0U;
		mTimerList["CheckConnectWifi"].first = millis();
		mStateConnect = STATE_CONNECT::NONE;
		mControlMode = CONTROL_MODE::DISPLAY_ALL;
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_NTP_FAILED):
	{
		LOGW("Set mFlagStatusNTP = 0");
		mFlagStatusNTP = 0;
		break;
	}
	default:
	{
		LOGW("Signal is not supported yet.");
		break;
	}
	}
}

void RemoteLight::run()
{
	mSERIAL->listenning();
	mIR->listenning();
	mBTN->listenning();

	switch (mControlMode)
	{
	case (CONTROL_MODE::CHECK_CONNECT_WIFI): {
		connectWifiMode();
		break;
	}
	case (CONTROL_MODE::DISPLAY_ALL): {
		displayAllTime();
		break;
	}
	case (CONTROL_MODE::SETUP_MODE): {
		displaySetupMode();
		break;
	}
	case (CONTROL_MODE::END_SETUP_MODE): {
		displayEndSetupMode();
		break;
	}
	default:
		break;
	}

	// delay(20);
}

void RemoteLight::connectWifiMode()
{
	// Include checking wifi, firebase
	auto &timer = mTimerList["CheckConnectWifi"];
	mNow = millis();
	mElapsed = mNow - timer.first;
	if (mElapsed > timer.second.first)
	{
		if ((mFlagStatusWifi == -1 || mFlagStatusWifi == 0) && mStateConnect == STATE_CONNECT::WIFI)
		{
			mSERIAL->handleSignal(SignaLType::SERIAL_CHECK_STATUS_WIFI);
		}
		else if ((mFlagStatusFirebase == -1 || mFlagStatusFirebase == 0) && mStateConnect == STATE_CONNECT::FIREBASE)
		{
			mSERIAL->handleSignal(SignaLType::SERIAL_CHECK_STATUS_FIREBASE);
		}
		else if ((mFlagStatusNTP == -1 || mFlagStatusNTP == 0) && mStateConnect == STATE_CONNECT::NTP)
		{
			mSERIAL->handleSignal(SignaLType::SERIAL_CHECK_STATUS_NTP);
		}
		else
		{
			// Do nothing
		}

		mLCD->handleSignal(SignaLType::LCD_DISPLAY_CONNECT_WIFI);
		timer.first = mNow;
		LOGD("Connect counter %d", timer.second.second.first);
		timer.second.second.first += 1;
		if (timer.second.second.first > timer.second.second.second)
		{
			if ((mFlagStatusWifi == -1 || mFlagStatusWifi == 0) && mStateConnect == STATE_CONNECT::WIFI)
			{
				LOGW("Timeout, Wifi connection failed. Cannot continue");
				mLCD->handleSignal(SignaLType::LCD_CONNECT_WIFI_FAILED);
				delay(1000);
				mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
				timer.second.second.first = 0U;
				mControlMode = CONTROL_MODE::DISPLAY_ALL;
				mStateConnect = STATE_CONNECT::NONE;
			}
			if ((mFlagStatusFirebase == -1 || mFlagStatusFirebase == 0) && mStateConnect == STATE_CONNECT::FIREBASE)
			{
				LOGW("Timeout, Firebase connection failed");
				mLCD->handleSignal(SignaLType::LCD_CONNECT_FIREBASE_FAILED);
				delay(1000);
				mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
				mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_NTP);
				timer.first = millis();
				timer.second.second.first = 0U;
				mStateConnect = STATE_CONNECT::NTP;
			}
			else if ((mFlagStatusNTP == -1 || mFlagStatusNTP == 0) && mStateConnect == STATE_CONNECT::NTP)
			{
				LOGW("Timeout, NTP connection failed");
				mLCD->handleSignal(SignaLType::LCD_CONNECT_NTP_FAILED);
				delay(1000);
				mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
				timer.first = millis();
				timer.second.second.first = 0U;
				mControlMode = CONTROL_MODE::DISPLAY_ALL;
				mStateConnect = STATE_CONNECT::NONE;
			}
		}
	}
}

void RemoteLight::displayAllTime()
{
	auto &timer = mTimerList["DisplayTimeAll"];
	mNow = millis();
	mElapsed = mNow - timer.first;
	if (mElapsed > timer.second.first)
	{
		timer.first = mNow;
		LOGD("counter %d", timer.second.second.first);
		timer.second.second.first += 1;
		mRTC->handleSignal(SignaLType::RTC_DISPLAY_ALL_TIME);
		if (timer.second.second.first > timer.second.second.second)
		{
			// reset
			mControlMode = CONTROL_MODE::NONE;
			timer.second.second.first = 0U;
			mLCD->handleSignal(SignaLType::LCD_CLEAR_TURN_OFF_SCREEN);
		}
	}
}

void RemoteLight::displaySetupMode()
{
	auto &timer = mTimerList["DisplaySetUpMode"];
	mNow = millis();
	mElapsed = mNow - timer.first;
	if (mElapsed > timer.second.first)
	{
		mControlMode = CONTROL_MODE::INTO_SETUP_MODE;
		mRTC->handleSignal(SignaLType::RTC_DISPLAY_ALL_TIME);
	}
}

void RemoteLight::displayEndSetupMode()
{
	auto &timer = mTimerList["DisplaySetUpMode"];
	mNow = millis();
	mElapsed = mNow - timer.first;
	if (mElapsed > timer.second.first)
	{
		// go to display all
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mControlMode = CONTROL_MODE::DISPLAY_ALL;
		mTimerList["DisplayTimeAll"].first = millis();
		mTimerList["DisplayTimeAll"].second.second.first = 0;
	}
}

