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
		{CONTROL_MODE::WIFI_PROVISIONING_MODE, 		SignalType::TASKS_WIFI_PROVISIONING_START},
		// {CONTROL_MODE::DISPLAY_TEMP_PRESS, 			std::make_pair(SignalType::TAKS_DISPLAY_TEMP_PRESS, "TASK DISPLAY TEMP PRESS")},
	};
	mCurrentControlMode = CONTROL_MODE::NONE;
	LOGI(" ================== RemoteLight ================== ");

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

	setControlMode(CONTROL_MODE::NONE);
	setCheckConfiguredTimeForLight(false);
	mRTC->init();
	mLCD->init();
	mEEPROM->init();
	mBTN->init();

	mEEPROM->handleSignal(SignalType::EEPROM_SEND_LIGHT_TIME_ON_OFF_DATA_TO_RTC);
	mEEPROM->handleSignal(SignalType::EEPROM_IS_STORED_SSID_PASSWORD);
	mRTC->handleSignal(SignalType::RTC_GET_ALL_ALL);
	// mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_WIFI);
	setControlMode(CONTROL_MODE::DISPLAY_ALL);
}

void RemoteLight::run()
{
	// mIR->listenning();
	// mBTN->listenning();
	mSerial->listenning();
	handleControlMode();
	handleTimeout();

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
	case (SignalType::BTN_PRESS_BTN_1_SIGNAL):
	case (SignalType::BTN_PRESS_BTN_2_SIGNAL):
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
	case (SignalType::LCD_START_PROVISIONING):
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
	case (SignalType::LCD_PROVISIONING_FAILED):
	case (SignalType::LCD_PROVISIONING_SUCCESS):
	case (SignalType::LCD_CLEAR_SCREEN):
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
	case (SignalType::NETWORK_SSID_PASSWORD_STORED):
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
		if(PRESS_BTN_1_2_COMBO_SIGNAL == signal)
		{
			if (mCurrentControlMode == CONTROL_MODE::DISPLAY_ALL)
			{
				LOGD("Stop DISPLAY ALL control mode due to PRESS BTN 1+2 COMBO SIGNAL");
				mTimerDisplayAll->stopTimer();
				mTasks->handleSignal(SignalType::TASKS_STOP_DISPLAY_ALL_TIME);
			}
			setControlMode(CONTROL_MODE::WIFI_PROVISIONING_MODE);
		}
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
	case (SignalType::EEPROM_NETWORK_SEND_SSID_PASSWORD): {
		mEEPROM->handleSignal(signal, data);
		break;
	}
	case (SignalType::REMOTE_LIGHT_REMOVE_WIFI_PROVISIONING_MODE): {
		mTimerConnectWifi->updateTimer([this]()
										{ this->onTimeout(SignalType::TIMER_CONNECT_FIREBASE_SIGNAL); }, DELAY_1S);
		mTimerConnectWifi->startTimer();
		setControlMode(CONTROL_MODE::CHECK_CONNECT_FIREBASE);
		break;
	}
	case (SignalType::REMOTE_LIGHT_REMOVE_CONNECT_FIREBASE_MODE): {
		mTimerConnectWifi->stopTimer();
		mTimerConnectWifi->updateTimer([this]()
										{ this->onTimeout(SignalType::TIMER_CONNECT_NTP_SIGNAL); }, DELAY_1S);
		mTimerConnectWifi->startTimer();
		setControlMode(CONTROL_MODE::CHECK_CONNECT_NTP);
		break;
	}
	case (SignalType::REMOTE_LIGHT_REMOVE_CONNECT_NTP_MODE): {
		mTimerConnectWifi->stopTimer();
		if(mPreviousControlMode == CONTROL_MODE::DISPLAY_ALL)
		{
			setControlMode(mPreviousControlMode);
			mPreviousControlMode = CONTROL_MODE::NONE;
			mTimerDisplayAll->startTimer();
		}
		break;
	}
	case (SignalType::REMOTE_LIGHT_REMOVE_DISPLAY_ALL_TIME_MODE): {
		mTimerDisplayAll->stopTimer();
		mQueueModeControl.pop();
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
		break;
	}
	case (SignalType::REMOTE_LIGHT_TIMER_CONNECT_FIREBASE_TIMEOUT): {
		mTimerConnectWifi->updateTimer([this]()
											{ this->onTimeout(SignalType::TIMER_CONNECT_NTP_SIGNAL); }, DELAY_1S);
		mTimerConnectWifi->startTimer();
		setControlMode(CONTROL_MODE::CHECK_CONNECT_NTP);
		break;
	}
	case (SignalType::REMOTE_LIGHT_TIMER_DISPLAY_ALLLTIME_START): {
		mTimerDisplayAll->startTimer();
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

void RemoteLight::setControlMode(const CONTROL_MODE state)
{
	std::unique_lock<std::mutex> lock(mMutexControlMode);
	if (state == CONTROL_MODE::NONE)
	{
		mControlMode = state;
	}
	else
	{
		if (mPreviousControlMode == CONTROL_MODE::NONE)
		{
			mPreviousControlMode = state;
			mControlMode = state;
		}
		else
		{
			if (mPreviousControlMode != state)
			{
				mPreviousControlMode = mControlMode
				mControlMode = state;
			}
			else
			{
				mControlMode = state;
			}
		}
	}
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
			LOGI("Task: %s", SIGNALTOSTRING(mControlModeSignalMap.at(modeCurrent)).c_str());
			mTasks->handleSignal(mControlModeSignalMap.at(modeCurrent));
		}
		else {
			LOGW("Task is not supported yet.");
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

int RemoteLight::getPriorityControlMode(const CONTROL_MODE mode) {
	switch (mode) {
		case CONTROL_MODE::DISPLAY_ALL:
			return 1;
		case CONTROL_MODE::WIFI_PROVISIONING_MODE:
			return 2;
		case CONTROL_MODE::CHECK_CONNECT_WIFI:
			return 2;
		case CONTROL_MODE::CHECK_CONNECT_FIREBASE:
			return 2;
		case CONTROL_MODE::CHECK_CONNECT_NTP:
			return 2;
		default:
			return 0;
	}
}