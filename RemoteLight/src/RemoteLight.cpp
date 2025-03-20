#include "./RemoteLight.h"

RemoteLight::RemoteLight()
{
	mSerial = std::make_shared<SerialPartner>(shared_from_this());
	mRTC = std::make_shared<RTC>(shared_from_this());
	mIR = std::make_shared<IRRemotePartner>(shared_from_this());
	mLCD = std::make_shared<LCD16x2>(shared_from_this());
	mBTN = std::make_shared<Button>(shared_from_this());
	mLIGHT = std::make_shared<Light>(shared_from_this());
	mNetwork = std::make_shared<Network>(shared_from_this());

	mTasks = std::make_shared<Tasks>(shared_from_this(), mLCD, mRTC, mIR, mNetwork);

	setControlMode(CONTROL_MODE::NONE);

	setCheckConfiguredTimeForLight(false);

	mFlagConnectNTP = 0;
	mFlagUpdateRTC = 0;

	mTimerMgr = std::make_shared<TimerManager>(4);
	mTimerConnectWifi = mTimerMgr->createTimer([this]()
											   { this->onTimeout(SignalType::TIMER_CONNECT_WIFI_SIGNAL); }, DELAY_3S);

	mTimerDisplayAll = mTimerMgr->createTimer([this]()
											  { this->onTimeout(SignalType::TIMER_DISPLAY_ALL_TIME_SIGNAL); }, DELAY_1S);

	mTimerDisplaySetupMode = mTimerMgr->createTimer([this]()
													{ this->onTimeout(SignalType::TIMER_DISPLAY_ALL_SETUP_MODE_SIGNAL); }, DELAY_3S);

	mTimerCheckConfiguredTimeForLight = mTimerMgr->createTimer([this]()
															   { this->onTimeout(SignalType::TIMER_CHECK_CONFIGURED_TIME_FOR_LIGHT); }, DELAY_1S);

	mControlModeSignalMap = {
		{CONTROL_MODE::CHECK_CONNECT_WIFI, 			std::make_pair(SignalType::TASKS_CONNECT_WIFI, 		"TASKS_CONNECT_WIFI")},
		{CONTROL_MODE::CHECK_CONNECT_FIREBASE, 		std::make_pair(SignalType::TASKS_CONNECT_FIREBASE, 	"TASKS_CONNECT_FIREBASE")},
		{CONTROL_MODE::CHECK_CONNECT_NTP, 			std::make_pair(SignalType::TASKS_CONNECT_NTP, 		"TASKS_CONNECT_NTP")},
		{CONTROL_MODE::DISPLAY_ALL, 				std::make_pair(SignalType::TASKS_DISPLAY_ALL_TIME, 	"TASKS_DISPLAY_ALL_TIME")},
		{CONTROL_MODE::START_SETUP_MODE, 			std::make_pair(SignalType::TASKS_START_SETUP_MODE, 	"TASKS_START_SETUP_MODE")},
		{CONTROL_MODE::END_SETUP_MODE, 				std::make_pair(SignalType::TASKS_END_SETUP_MODE, 	"TASKS_END_SETUP_MODE")},
	};
	LOGI("Initialization RemoteLight!");
}

RemoteLight::~RemoteLight()
{
}

void RemoteLight::init()
{
	mRTC->handleSignal(SignalType::RTC_GET_ALL_ALL);
	mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_WIFI);
	setControlMode(CONTROL_MODE::CHECK_CONNECT_WIFI);
	mTimerCheckConfiguredTimeForLight->startTimer();
}

void RemoteLight::run()
{
	mSerial->listenning();
	mIR->listenning();
	mBTN->listenning();

	handleControlMode();
	handleTimeout();

	if (getCheckConfiguredTimeForLight() == true)
	{
		setCheckConfiguredTimeForLight(false);
		mRTC->handleSignal(SignalType::RTC_CHECK_CONFIGURED_TIME_FOR_LIGHT);
		mTasks->handleSignal(SignalType::REMOTE_LIGHT_CHECK_COMMAND_FIREBASE);
		mTimerCheckConfiguredTimeForLight->startTimer();
	}
}

void RemoteLight::handleSignal(const SignalType signal, Package *data)
{
	// LOGD("Handle signal value: %d", signal);
	switch (signal)
	{
	case (SignalType::PRESS_BTN_1_SIGNAL):
	case (SignalType::PRESS_BTN_2_SIGNAL):
	case (SignalType::PRESS_BTN_3_SIGNAL):
	{
		mTasks->handleSignal(SignalType::TASKS_INSTALL_IR_BUTTON);
	}
	case (SignalType::IR_BTN_1_SIGNAL):
	case (SignalType::IR_BTN_2_SIGNAL):
	case (SignalType::IR_BTN_3_SIGNAL):
	case (SignalType::IR_BTN_4_SIGNAL):
	case (SignalType::RTC_TURN_ON_LIGHT1):
	case (SignalType::RTC_TURN_ON_LIGHT2):
	case (SignalType::RTC_TURN_ON_LIGHT3):
	case (SignalType::RTC_TURN_ON_LIGHT4):
	case (SignalType::RTC_TURN_OFF_LIGHT1):
	case (SignalType::RTC_TURN_OFF_LIGHT2):
	case (SignalType::RTC_TURN_OFF_LIGHT3):
	case (SignalType::RTC_TURN_OFF_LIGHT4):
	{
		mLIGHT->handleSignal(signal);
		break;
	}
	case (SignalType::LCD_MENU_MODE_BACK):
	case (SignalType::LCD_MENU_MODE_OK):
	case (SignalType::LCD_MOVE_LEFT_MENU_MODE):
	case (SignalType::LCD_MOVE_RIGHT_MENU_MODE):
	case (SignalType::LCD_DISPLAY_ALL_TIME):
	case (SignalType::REMOTE_LIGHT_IRBUTTON_INSTALL):
	{
		mLCD->handleSignal(signal, data);
		break;
	}
	case (SignalType::LCD_DISPLAY_CONNECT_WIFI):
	case (SignalType::LCD_CONNECT_WIFI_SUCCESS):
	case (SignalType::LCD_CONNECT_FIREBASE_SUCCESS):
	case (SignalType::LCD_CONNECT_NTP_SUCCESS):
	case (SignalType::LCD_CONNECT_WIFI_FAILED):
	case (SignalType::LCD_CONNECT_FIREBASE_FAILED):
	case (SignalType::LCD_CONNECT_NTP_FAILED): {
		mLCD->handleSignal(signal);
		break;
	}
	case (SignalType::WEB_SET_ALLTIME_DATA_RESPONSE): {
		mNetwork->handleSignal(signal);
	}
	case (SignalType::IR_BTN_MENU_SIGNAL):
	case (SignalType::IR_BTN_APP_SIGNAL):
	case (SignalType::IR_BTN_UP_SIGNAL):
	case (SignalType::IR_BTN_DOWN_SIGNAL):
	case (SignalType::IR_BTN_LEFT_SIGNAL):
	case (SignalType::IR_BTN_RIGHT_SIGNAL):
	case (SignalType::IR_BTN_OK_SIGNAL):
	case (SignalType::IR_BTN_BACK_SIGNAL):
	case (SignalType::TASKS_CONNECT_WIFI_SUCCESS):
	case (SignalType::TASKS_CONNECT_WIFI_FAILED):
	case (SignalType::TASKS_CONNECT_FIREBASE_SUCCESS):
	case (SignalType::TASKS_CONNECT_FIREBASE_FAILED):
	case (SignalType::TASKS_CONNECT_NTP_SUCCESS):
	case (SignalType::TASKS_CONNECT_NTP_FAILED):
	case (SignalType::RTC_COUNTER_INSTALL_IRBUTTON_REACHED): {
		mTasks->handleSignal(signal);
		break;
	}
	case (SignalType::WEB_GET_ALLTIME_DATA_REQUEST):
	case (SignalType::WEB_GET_LIGHT1_DATA_REQUEST):
	case (SignalType::WEB_GET_LIGHT2_DATA_REQUEST):
	case (SignalType::WEB_GET_LIGHT3_DATA_REQUEST):
	case (SignalType::WEB_GET_LIGHT4_DATA_REQUEST): {
		mRTC->handleSignal(signal);
		break;
	}
	case (SignalType::WEB_GET_ALLTIME_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT1_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT2_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT3_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT4_DATA_RESPONSE):
	case (SignalType::WEB_GET_STATUS_DATA_RESPONSE): {
		mNetwork->handleSignal(signal, data);
		break;
	}
	case (SignalType::WEB_SET_ALLTIME_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT1_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT2_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT3_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT4_DATA_REQUEST):
	case (SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP): {
		mRTC->handleSignal(signal, data);
		break;
	}
	case (SignalType::WEB_SET_LIGHT1_DATA_RESPONSE):
	case (SignalType::WEB_SET_LIGHT2_DATA_RESPONSE):
	case (SignalType::WEB_SET_LIGHT3_DATA_RESPONSE):
	case (SignalType::WEB_SET_LIGHT4_DATA_RESPONSE):
	case (SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE):
	case (SignalType::NETWORK_GET_TIME_DATE_FROM_NTP): {
		mNetwork->handleSignal(signal);
		break;
	}
	case (SignalType::WEB_GET_STATUS_DATA_REQUEST): {
		mLIGHT->handleSignal(signal);
		break;
	}
	case (SignalType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST):
	case (SignalType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST):
	case (SignalType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST):
	case (SignalType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST): {
		mLIGHT->handleSignal(signal, data);
		break;
	}
	case (SignalType::IR_INSTALL_BUTTON_COMPLETE): {
		setControlMode(CONTROL_MODE::DISPLAY_ALL);
		break;
	}
	case (SignalType::REMOTE_LIGHT_CONNECT_WIFI): {
		setControlMode(CONTROL_MODE::CHECK_CONNECT_WIFI);
		break;
	}
	case (SignalType::REMOTE_LIGHT_CONNECT_FIREBASE): {
		setControlMode(CONTROL_MODE::CHECK_CONNECT_FIREBASE);
		break;
	}
	case (SignalType::REMOTE_LIGHT_CONNECT_NTP): {
		setControlMode(CONTROL_MODE::CHECK_CONNECT_NTP);
		break;
	}
	case (SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME): {
		setControlMode(CONTROL_MODE::DISPLAY_ALL);
		break;
	}
	case (SignalType::REMOTE_LIGHT_END_SETUP_MODE): {
		setControlMode(CONTROL_MODE::END_SETUP_MODE);
		break;
	}
	case (SignalType::REMOTE_LIGHT_INTO_MENU_MODE): {
		setControlMode(CONTROL_MODE::INTO_MENU_MODE);
		break;
	}
	case (SignalType::REMOTE_LIGHT_MENU_MODE): {
		setControlMode(CONTROL_MODE::MENU_MODE);
		break;
	}
	default: {
		LOGW("Signal is not supported yet.");
		break;
	}
	}
}

void RemoteLight::onTimeout(const SignalType signal)
{
	switch (signal)
	{
	case SignalType::TIMER_CONNECT_WIFI_SIGNAL:
	case SignalType::TIMER_CONNECT_WIFI_SUCCESS_GOTO_NEXT_CONNECT:
	case SignalType::TIMER_CONNECT_WIFI_FAILED_GOTO_NEXT_MODE:
	case SignalType::TIMER_CONNECT_FIREBASE_SIGNAL:
	case SignalType::TIMER_CONNECT_FIREBASE_SUCCESS_GOTO_NEXT_CONNECT:
	case SignalType::TIMER_CONNECT_FIREBASE_FAILED_GOTO_NEXT_CONNECT:
	case SignalType::TIMER_CONNECT_NTP_SIGNAL:
	case SignalType::TIMER_CONNECT_NTP_SUCCESS_GOTO_NEXT_CONNECT:
	case SignalType::TIMER_CONNECT_NTP_FAILED_GOTO_NEXT_CONNECT:
	case SignalType::TIMER_DISPLAY_ALL_TIME_SIGNAL:
	case SignalType::TIMER_DISPLAY_ALL_END_SETUP_MODE_SIGNAL:
	case SignalType::TIMER_DISPLAY_ALL_SETUP_MODE_SIGNAL:
	{
		mFlagTimeout = signal;
		break;
	}
	case SignalType::TIMER_CHECK_CONFIGURED_TIME_FOR_LIGHT:
	{
		setCheckConfiguredTimeForLight(true);
		break;
	}

	default:
		break;
	}
}

void RemoteLight::setControlMode(const CONTROL_MODE state)
{
	std::unique_lock<std::mutex> lock(mMutex);
	mControlMode = state;
}

RemoteLight::CONTROL_MODE RemoteLight::getControlMode()
{
	std::unique_lock<std::mutex> lock(mMutex);
	return mControlMode;
}

void RemoteLight::setCheckConfiguredTimeForLight(const bool state)
{
	mCheckConfiguredTimeForLight = state;
}

bool RemoteLight::getCheckConfiguredTimeForLight()
{
	return mCheckConfiguredTimeForLight;
}

void RemoteLight::handleControlMode()
{
	CONTROL_MODE modeCurrent = getControlMode();
	if (modeCurrent == CONTROL_MODE::NONE) {
		return;
	}
	else {
		if(mControlModeSignalMap.find(modeCurrent) != mControlModeSignalMap.end()) {
			mTasks->handleSignal(mControlModeSignalMap.at(modeCurrent).first);
			LOGI("Control mode: %s", mControlModeSignalMap.at(modeCurrent).second.c_str());
		}
		else {
			LOGW("Control mode is not supported yet.");
		}
		setControlMode(CONTROL_MODE::NONE);
	}
}

void RemoteLight::handleTimeout()
{
	if (mFlagTimeout == SignalType::NONE) {
		return;
	}
	else {
		switch (mFlagTimeout)
		{
		case SignalType::TIMER_CONNECT_WIFI_SIGNAL:
		case SignalType::TIMER_CONNECT_WIFI_SUCCESS_GOTO_NEXT_CONNECT:
		case SignalType::TIMER_CONNECT_WIFI_FAILED_GOTO_NEXT_MODE:
		case SignalType::TIMER_CONNECT_FIREBASE_SIGNAL:
		case SignalType::TIMER_CONNECT_FIREBASE_SUCCESS_GOTO_NEXT_CONNECT:
		case SignalType::TIMER_CONNECT_FIREBASE_FAILED_GOTO_NEXT_CONNECT:
		case SignalType::TIMER_CONNECT_NTP_SIGNAL:
		case SignalType::TIMER_CONNECT_NTP_SUCCESS_GOTO_NEXT_CONNECT:
		case SignalType::TIMER_CONNECT_NTP_FAILED_GOTO_NEXT_CONNECT:
		case SignalType::TIMER_DISPLAY_ALL_TIME_SIGNAL:
		case SignalType::TIMER_DISPLAY_ALL_END_SETUP_MODE_SIGNAL: {
			mTasks->handleSignal(mFlagTimeout);
			break;
		}
		case SignalType::TIMER_DISPLAY_ALL_SETUP_MODE_SIGNAL: {
			setControlMode(CONTROL_MODE::START_SETUP_MODE);
			break;
		}
		case SignalType::TIMER_CHECK_CONFIGURED_TIME_FOR_LIGHT: {
			setCheckConfiguredTimeForLight(true);
			break;
		}
		default:
			break;
		}
		mFlagTimeout = SignalType::NONE;
	}
}