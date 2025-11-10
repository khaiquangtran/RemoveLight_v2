#include "./RemoteLight.h"

RemoteLight::RemoteLight()
{
	mControlModeSignalMap = {
		{CONTROL_MODE::CHECK_CONNECT_WIFI, 			SignalType::TASKS_CONNECT_WIFI},
		{CONTROL_MODE::CHECK_CONNECT_FIREBASE, 		SignalType::TASKS_CONNECT_FIREBASE},
		{CONTROL_MODE::CHECK_CONNECT_NTP, 			SignalType::TASKS_CONNECT_NTP},
		{CONTROL_MODE::DISPLAY_ALL, 				SignalType::TASKS_DISPLAY_ALL_TIME},
		{CONTROL_MODE::START_SETUP_MODE, 			SignalType::TASKS_START_SETUP_MODE},
		{CONTROL_MODE::END_SETUP_MODE, 				SignalType::TASKS_END_SETUP_MODE},
		// {CONTROL_MODE::DISPLAY_TEMP_PRESS, 			std::make_pair(SignalType::TAKS_DISPLAY_TEMP_PRESS, "TASK DISPLAY TEMP PRESS")},
	};

	// mFlagIsStoredSsidPassword = false;
}

RemoteLight::~RemoteLight()
{
}

void RemoteLight::init()
{
	mSerial 	= std::make_shared<SerialPartner>(shared_from_this());
	mEEPROM 	= std::make_shared<EEPROMPartner>(shared_from_this());
	mRTC 		= std::make_shared<RTC>(shared_from_this());
	mIR 		= std::make_shared<IRRemotePartner>(shared_from_this());
	mLCD 		= std::make_shared<LCD16x2>(shared_from_this());
	mBTN 		= std::make_shared<Button>(shared_from_this());
	mLIGHT 		= std::make_shared<Light>(shared_from_this());
	// mBluetooth 	= std::make_shared<BluetoothPartner>(shared_from_this());
	mNetwork 	= std::make_shared<Network>(shared_from_this());

	mTasks = std::make_shared<Tasks>(shared_from_this(), mLCD, mRTC, mIR, mNetwork);

	mTimerMgr = std::make_shared<TimerManager>(4);
	mTimerConnectWifi = mTimerMgr->createTimer([this]()
												{ this->onTimeout(SignalType::TIMER_CONNECT_WIFI_SIGNAL); }, DELAY_3S);

	mTimerDisplayAll = mTimerMgr->createTimer([this]()
											  { this->onTimeout(SignalType::TIMER_DISPLAY_ALL_TIME_SIGNAL); }, DELAY_1S);

	mTimerDisplaySetupMode = mTimerMgr->createTimer([this]()
													{ this->onTimeout(SignalType::TIMER_DISPLAY_ALL_SETUP_MODE_SIGNAL); }, DELAY_3S);

	mTimerCheckConfiguredTimeForLight = mTimerMgr->createTimer([this]()
															   { this->onTimeout(SignalType::TIMER_CHECK_CONFIGURED_TIME_FOR_LIGHT); }, DELAY_1S);
	LOGI(" ================== RemoteLight ================== ");

	setControlMode(CONTROL_MODE::NONE);
	setCheckConfiguredTimeForLight(false);
	mRTC->init();
	mLCD->init();
	mEEPROM->init();
	mBTN->init();

	mEEPROM->handleSignal(SignalType::EEPROM_SEND_LIGHT_TIME_ON_OFF_DATA_TO_RTC);
	mRTC->handleSignal(SignalType::RTC_GET_ALL_ALL);
	mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_WIFI);
	setControlMode(CONTROL_MODE::DISPLAY_ALL);
}

void RemoteLight::run()
{
	// mIR->listenning();
	mBTN->listenning();
	handleControlMode();
	handleTimeout();
	mNetwork->listenBluetoothData();

	// if (getCheckConfiguredTimeForLight() == true)
	// {
		// setCheckConfiguredTimeForLight(false);
		// mRTC->handleSignal(SignalType::RTC_CHECK_CONFIGURED_TIME_FOR_LIGHT);
		// mTasks->handleSignal(SignalType::REMOTE_LIGHT_CHECK_COMMAND_FIREBASE);
		// mTimerCheckConfiguredTimeForLight->startTimer();
	// }
}

void RemoteLight::handleSignal(const SignalType signal, Package *data)
{
	// LOGD("Handle signal value: %d", signal);
	switch (signal)
	{
	case (SignalType::PRESS_BTN_1_SIGNAL):
	case (SignalType::PRESS_BTN_2_SIGNAL):
	// {
	// 	mTasks->handleSignal(SignalType::TASKS_INSTALL_IR_BUTTON);
	// }
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
	case (SignalType::LCD_CONNECT_NTP_FAILED):
	case (SignalType::LCD_BLUETOOTH_CONNECTED_SUCCESS):
	case (SignalType::LCD_BLUETOOTH_CONNECTED_FAILED):
	{
		mLCD->handleSignal(signal);
		break;
	}
	case (SignalType::WEB_SET_ALLTIME_DATA_RESPONSE): {
		mNetwork->handleSignal(signal);
		break;
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
		// mRTC->handleSignal(signal);
		break;
	}
	case (SignalType::WEB_GET_ALLTIME_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT1_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT2_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT3_DATA_RESPONSE):
	case (SignalType::WEB_GET_LIGHT4_DATA_RESPONSE):
	case (SignalType::WEB_GET_STATUS_DATA_RESPONSE):
	{
		mNetwork->handleSignal(signal, data);
		break;
	}
	case (SignalType::WEB_SET_ALLTIME_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT1_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT2_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT3_DATA_REQUEST):
	case (SignalType::WEB_SET_LIGHT4_DATA_REQUEST):
	case (SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP):
	case (SignalType::RTC_GET_LIGHT_ON_OFF_DATA):
	{
		mRTC->handleSignal(signal, data);
		break;
	}
	case (SignalType::WEB_SET_LIGHT1_DATA_RESPONSE):
	case (SignalType::WEB_SET_LIGHT2_DATA_RESPONSE):
	case (SignalType::WEB_SET_LIGHT3_DATA_RESPONSE):
	case (SignalType::WEB_SET_LIGHT4_DATA_RESPONSE):
	case (SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE):
	case (SignalType::NETWORK_GET_TIME_DATE_FROM_NTP):
	case (SignalType::PRESS_BTN_1_2_COMBO_SIGNAL):
	{
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
	case (SignalType::IR_ERRPROM_SEND_DATA): {
		mIR->handleSignal(signal, data);
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
	case (SignalType::REMOTE_LIGHT_END_SETUP_MODE): {
		setControlMode(CONTROL_MODE::END_SETUP_MODE);
		break;
	}
	case (SignalType::REMOTE_LIGHT_INTO_MENU_MODE):
	{
		setControlMode(CONTROL_MODE::INTO_MENU_MODE);
		break;
	}
	case (SignalType::REMOTE_LIGHT_MENU_MODE):
	{
		setControlMode(CONTROL_MODE::MENU_MODE);
		break;
	}
	case (SignalType::REMOTE_LIGHT_TIMER_CONNECT_WIFI_START):
	{
		mTimerConnectWifi->startTimer();
		break;
	}
	case (SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME):
	case (SignalType::REMOTE_LIGHT_TIMER_CONNECT_NTP_TIMEOUT):
	case (SignalType::REMOTE_LIGHT_TIMER_CONNECT_WIFI_TIMEOUT):
	{
		setControlMode(CONTROL_MODE::DISPLAY_ALL);
		mTimerDisplayAll->startTimer();
		break;
	}
	case (SignalType::REMOTE_LIGHT_TIMER_CONNECT_FIREBASE_TIMEOUT): {
		setControlMode(CONTROL_MODE::CHECK_CONNECT_NTP);
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
	case SignalType::TIMER_DISPLAY_TEMP_PRESS_SIGNAL: {
		setFlagTimeout(signal);
		break;
	}
	case SignalType::TIMER_CHECK_CONFIGURED_TIME_FOR_LIGHT: {
		setCheckConfiguredTimeForLight(true);
		break;
	}
	default:
		break;
	}
}

void RemoteLight::setControlMode(const CONTROL_MODE state) {
	std::unique_lock<std::mutex> lock(mMutexControlMode);
	mControlMode = state;
}

RemoteLight::CONTROL_MODE RemoteLight::getControlMode() {
	std::unique_lock<std::mutex> lock(mMutexControlMode);
	return mControlMode;
}

void RemoteLight::setCheckConfiguredTimeForLight(const bool state) {
	mCheckConfiguredTimeForLight = state;
}

bool RemoteLight::getCheckConfiguredTimeForLight() {
	return mCheckConfiguredTimeForLight;
}

void RemoteLight::handleControlMode() {
	CONTROL_MODE modeCurrent = getControlMode();
	if (modeCurrent == CONTROL_MODE::NONE) {
		return;
	}
	else {
		if(mControlModeSignalMap.find(modeCurrent) != mControlModeSignalMap.end()) {
			LOGI("Control mode: %s", SIGNALTOSTRING(mControlModeSignalMap.at(modeCurrent)).c_str());
			mTasks->handleSignal(mControlModeSignalMap.at(modeCurrent));
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
		setFlagTimeout(SignalType::NONE);
	}
}

void RemoteLight::setFlagTimeout(SignalType signal) {
	std::unique_lock<std::mutex> lock(mMutex);
	mFlagTimeout = signal;
}

SignalType RemoteLight::getFlagTimeout() {
	std::unique_lock<std::mutex> lock(mMutex);
	return mFlagTimeout;
}