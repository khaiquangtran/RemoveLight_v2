#include "./Tasks.h"

Tasks::Tasks(std::shared_ptr<RemoteLight> rml, std::shared_ptr<Hardware> lcd, std::shared_ptr<Hardware> rtc, std::shared_ptr<Hardware> ir, std::shared_ptr<Network> net) :
    mRML(rml), mLCD(lcd), mNET(net), mRTC(rtc), mIR(ir)
{
    mCounterConnectWifi = 0U;
    mFlagInstallIRButton = 0;
    mFlagConnectFirebase = CONNECT_STATUS::UNKNOWN;
    mModeHandle = MODE_HANDLE::NONE;
}

void Tasks::handleSignal(const SignalType signal, Package *data)
{
    switch (signal) {
    case (SignalType::TASKS_CONNECT_WIFI): {
        connectWifiMode();
        break;
    }
    case (SignalType::TASKS_CONNECT_WIFI_SUCCESS): {
        mRML->mTimerConnectWifi->stopTimer();
        mCounterConnectWifi = 0;
        LOGI("WIFI connection SUCCESS!");
        mRML->handleSignal(SignalType::LCD_CONNECT_WIFI_SUCCESS);
        mRML->mTimerConnectWifi->updateTimer(
            [this]() { mRML->onTimeout(SignalType::TIMER_CONNECT_WIFI_SUCCESS_GOTO_NEXT_CONNECT); }, DELAY_5S);
        mRML->mTimerConnectWifi->startTimer();
        break;
    }
    case (SignalType::TASKS_CONNECT_WIFI_FAILED): {
        LOGW("WIFI connection FAILED!");
        break;
    }
    case (SignalType::TIMER_CONNECT_WIFI_SIGNAL): {
        connectWifiTimeout();
        break;
    }
    case (SignalType::TIMER_CONNECT_WIFI_SUCCESS_GOTO_NEXT_CONNECT): {
        mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_FIREBASE);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_FIREBASE);
        break;
    }
    case (SignalType::TIMER_CONNECT_WIFI_FAILED_GOTO_NEXT_MODE): {
        mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
        break;
    }
    case (SignalType::TASKS_CONNECT_FIREBASE): {
        connectFirebaseMode();
        break;
    }
    case (SignalType::TASKS_CONNECT_FIREBASE_SUCCESS): {
        mRML->mTimerConnectWifi->stopTimer();
        mCounterConnectWifi = 0;
        mFlagConnectFirebase = CONNECT_STATUS::SUCCESS;
        LOGI("FIREBASE connection SUCCESS!");
        mRML->handleSignal(SignalType::LCD_CONNECT_FIREBASE_SUCCESS);
        mRML->mTimerConnectWifi->updateTimer(
            [this]() { mRML->onTimeout(SignalType::TIMER_CONNECT_FIREBASE_SUCCESS_GOTO_NEXT_CONNECT); }, DELAY_5S);
        mRML->mTimerConnectWifi->startTimer();
        break;
    }
    case (SignalType::TASKS_CONNECT_FIREBASE_FAILED): {
        LOGW("FIREBASE connection FAILED!");
        mFlagConnectFirebase = CONNECT_STATUS::FAILED;
        break;
    }
    case (SignalType::TIMER_CONNECT_NTP_SIGNAL): {
        connectFirebaseTimeout();
        break;
    }
    case (SignalType::TIMER_CONNECT_FIREBASE_SUCCESS_GOTO_NEXT_CONNECT):
    case (SignalType::TIMER_CONNECT_FIREBASE_FAILED_GOTO_NEXT_CONNECT): {
        mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_NTP);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_NTP);
        break;
    }
    case (SignalType::TASKS_CONNECT_NTP): {
        connectNTPMode();
        break;
    }
    case (SignalType::TASKS_CONNECT_NTP_SUCCESS): {
        mRML->mTimerConnectWifi->stopTimer();
        mCounterConnectWifi = 0;
        LOGI("NTP connection SUCCESS!");
        mLCD->handleSignal(SignalType::LCD_CONNECT_NTP_SUCCESS);
        mNET->handleSignal(SignalType::NETWORK_GET_TIME_DATE_FROM_NTP);
        mRML->mTimerConnectWifi->updateTimer(
            [this]() { mRML->onTimeout(SignalType::TIMER_CONNECT_NTP_SUCCESS_GOTO_NEXT_CONNECT); }, DELAY_5S);
        mRML->mTimerConnectWifi->startTimer();
        break;
    }
    case (SignalType::TASKS_CONNECT_NTP_FAILED): {
        LOGW("NTP connection FAILED!");
        break;
    }
    case (SignalType::TASKS_CONNECT_NTP_TIMEOUT): {
        connectNTPTimeout();
        break;
    }
    case (SignalType::TIMER_CONNECT_NTP_SUCCESS_GOTO_NEXT_CONNECT):
    case (SignalType::TIMER_CONNECT_NTP_FAILED_GOTO_NEXT_CONNECT): {
        mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
        break;
    }
    case (SignalType::TASKS_DISPLAY_ALL_TIME): {
        displayAllTime();
        break;
    }
    case (SignalType::TIMER_DISPLAY_ALL_TIME_SIGNAL): {
        displayAllTimeTimeout();
        break;
    }
    case (SignalType::TIMER_DISPLAY_ALL_END_SETUP_MODE_SIGNAL): {
		mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
        break;
    }
    case (SignalType::IR_BTN_APP_SIGNAL): {
        displayReadySetupMode();
        break;
    }
    case (SignalType::TASKS_START_SETUP_MODE): {
        intoSetupMode();
        break;
    }
    case (SignalType::TASKS_END_SETUP_MODE): {
        displayEndSetupMode();
        break;
    }
    case (SignalType::IR_BTN_UP_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			mRTC->handleSignal(SignalType::RTC_INCREASE_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			mRTC->handleSignal(SignalType::RTC_INCREASE_VALUE_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_DOWN_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			mRTC->handleSignal(SignalType::RTC_DECREASE_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			mRTC->handleSignal(SignalType::RTC_DECREASE_VALUE_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_LEFT_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			mRTC->handleSignal(SignalType::RTC_SHIFT_LEFT_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
			mRTC->handleSignal(SignalType::RTC_MOVE_LEFT_MENU_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			mRTC->handleSignal(SignalType::RTC_MOVE_LEFT_INTO_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_RIGHT_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			mRTC->handleSignal(SignalType::RTC_SHIFT_RIGHT_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
			mRTC->handleSignal(SignalType::RTC_MOVE_RIGHT_MENU_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			mRTC->handleSignal(SignalType::RTC_MOVE_RIGHT_INTO_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_OK_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			mRTC->handleSignal(SignalType::RTC_SETUP_MODE_OK);
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			mLCD->handleSignal(SignalType::LCD_DISPLAY_END_SETUP_MODE);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_END_SETUP_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
            mRML->handleSignal(SignalType::REMOTE_LIGHT_INTO_MENU_MODE);
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			mRTC->handleSignal(SignalType::RTC_MENU_MODE_OK);
		}
		break;
	}
    case (SignalType::IR_BTN_MENU_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::NONE) {
			mRML->mTimerDisplayAll->stopTimer();
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
			mLCD->handleSignal(SignalType::IR_BTN_MENU_SIGNAL);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_BACK_SIGNAL): {
        if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			mRTC->handleSignal(SignalType::RTC_BACK_MENU_MODE);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_MENU_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
		}
        break;
    }
    case (SignalType::REMOTE_LIGHT_CHECK_COMMAND_FIREBASE): {
        if(mFlagConnectFirebase == CONNECT_STATUS::SUCCESS) {
            mNET->handleSignal(SignalType::REMOTE_LIGHT_CHECK_COMMAND_FIREBASE);
        }
        break;
    }
    case (SignalType::TASKS_INSTALL_IR_BUTTON): {
        installIRButton();
        break;
    }
    case (SignalType::RTC_COUNTER_INSTALL_IRBUTTON_REACHED): {
		LOGW("Install IR button mode is timeout!!!");
		mFlagInstallIRButton = 0;
		LOGI("mFlagInstallIRButton reset: %d", mFlagInstallIRButton);
		break;
	}
    case (SignalType::WEB_SET_ALLTIME_DATA_RESPONSE): {
        if (mCounterDisplayAllTime > 25 || mCounterDisplayAllTime == 0) {
			mCounterDisplayAllTime = 0;
			mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
		}
        break;
    }
    default: break;
    }
}

void Tasks::connectWifiMode() {
    mLCD->handleSignal(SignalType::LCD_DISPLAY_CONNECT_WIFI);
    LOGI("WIFI connection %d times", mCounterConnectWifi);
	mNET->handleSignal(SignalType::NETWORK_CHECK_STATUS_WIFI);
    mRML->mTimerConnectWifi->startTimer();
}

void Tasks::connectFirebaseMode() {
    mLCD->handleSignal(SignalType::LCD_DISPLAY_CONNECT_WIFI);
    LOGI("FIREBASE connection %d times", mCounterConnectWifi);
    mRML->mTimerConnectWifi->updateTimer([this]()
                                   { mRML->onTimeout(SignalType::TIMER_CONNECT_FIREBASE_SIGNAL); }, DELAY_5S);
    mRML->mTimerConnectWifi->startTimer();
    mNET->handleSignal(SignalType::NETWORK_CHECK_STATUS_FIREBASE);
}

void Tasks::connectNTPMode() {
    mLCD->handleSignal(SignalType::LCD_DISPLAY_CONNECT_WIFI);
    LOGI("NTP connection %d times", mCounterConnectWifi);
    mNET->handleSignal(SignalType::NETWORK_CHECK_STATUS_NTP);
    mRML->mTimerConnectWifi->updateTimer([this]()
                                   { mRML->onTimeout(SignalType::TIMER_CONNECT_NTP_SIGNAL); }, DELAY_3S);
    mRML->mTimerConnectWifi->startTimer();
}

void Tasks::connectWifiTimeout()
{
    if (mCounterConnectWifi < REPEATS_10) {
        mCounterConnectWifi++;
        mRML->mTimerConnectWifi->startTimer();
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_WIFI);
    }
    else {
        mCounterConnectWifi = 0;
        mRML->handleSignal(SignalType::LCD_CONNECT_WIFI_FAILED);
        LOGW("WIFI connection FAILED!");
        mRML->mTimerConnectWifi->updateTimer(
            [this]() { mRML->onTimeout(SignalType::TIMER_CONNECT_WIFI_FAILED_GOTO_NEXT_MODE); }, DELAY_5S);
        mRML->mTimerConnectWifi->startTimer();
    }
}

void Tasks::connectFirebaseTimeout()
{
    if (mCounterConnectWifi < REPEATS_10) {
        mCounterConnectWifi++;
        mRML->mTimerConnectWifi->startTimer();
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_FIREBASE);
    }
    else {
        mCounterConnectWifi = 0;
        mLCD->handleSignal(SignalType::LCD_CONNECT_FIREBASE_FAILED);
        LOGW("Firebase connection FAILED!");
        mRML->mTimerConnectWifi->updateTimer(
            [this]() { mRML->onTimeout(SignalType::TIMER_CONNECT_FIREBASE_FAILED_GOTO_NEXT_CONNECT); }, DELAY_5S);
        mRML->mTimerConnectWifi->startTimer();
    }
}

void Tasks::connectNTPTimeout()
{
    if (mCounterConnectWifi < REPEATS_10) {
		mCounterConnectWifi++;
        mRML->mTimerConnectWifi->startTimer();
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_NTP);
	}
	else {
		mCounterConnectWifi = 0;
		mLCD->handleSignal(SignalType::LCD_CONNECT_NTP_FAILED);
		LOGW("NTP connection FAILED!");
        mRML->mTimerConnectWifi->updateTimer(
            [this]() { mRML->onTimeout(SignalType::TIMER_CONNECT_NTP_FAILED_GOTO_NEXT_CONNECT); }, DELAY_5S);
        mRML->mTimerConnectWifi->startTimer();
	}
}

void Tasks::displayAllTime()
{
	mRTC->handleSignal(SignalType::RTC_DISPLAY_ALL_TIME);
	mRML->mTimerDisplayAll->startTimer();
}

void Tasks::displayAllTimeTimeout()
{
    if (mCounterDisplayAllTime < REPEATS_30) {
		mCounterDisplayAllTime++;
        mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
	}
	else {
		mCounterDisplayAllTime = 0;
		mLCD->handleSignal(SignalType::LCD_CLEAR_TURN_OFF_SCREEN);
	}
}

void Tasks::displayReadySetupMode()
{
    mLCD->handleSignal(SignalType::IR_BTN_APP_SIGNAL);
	mRML->mTimerDisplayAll->stopTimer();
	mRML->mTimerDisplaySetupMode->startTimer();
}

void Tasks::intoSetupMode()
{
    mModeHandle = MODE_HANDLE::INTO_SETUP_MODE;
	mRTC->handleSignal(SignalType::RTC_DISPLAY_ALL_TIME);
}

void Tasks::displayEndSetupMode()
{
    mModeHandle = MODE_HANDLE::NONE;
	mRML->mTimerDisplaySetupMode->updateTimer(
		[this]()
		{ mRML->onTimeout(SignalType::TIMER_DISPLAY_ALL_END_SETUP_MODE_SIGNAL); }, DELAY_3S);
    mRML->mTimerDisplaySetupMode->startTimer();
}

void Tasks::installIRButton()
{
    if (mModeHandle == MODE_HANDLE::NONE)
	{
		if (mFlagInstallIRButton == 0) {
			LOGI("Start the timer for IR button mode installation!");
			mRTC->handleSignal(SignalType::RTC_COUNTER_INSTALL_IRBUTTON);
		}
		mFlagInstallIRButton++;
		LOGI("mFlagInstallIRButton: %d", mFlagInstallIRButton);
		if (mFlagInstallIRButton == 6) {
			LOGI("mFlagInstallIRButton reached 6 times. Go to IR button mode installation.");
			mFlagInstallIRButton = 0;
			mRTC->handleSignal(SignalType::REMOTE_LIGHT_PRESS_BUTTON_REACHED_MAX_TIME);
			mModeHandle = MODE_HANDLE::INSTALL_IR_BUTTON;
			mIR->handleSignal(SignalType::IR_INSTALL_BUTTON);
			mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
			mLCD->handleSignal(SignalType::LCD_INSTALL_BUTTON1);
		}
	}
	else if (mModeHandle == MODE_HANDLE::INSTALL_IR_BUTTON) {
		LOGI("Install button Done. Move next button");
		mIR->handleSignal(SignalType::IR_INSTALL_BUTTON_DONE);
	}
    else {
        // Do nothing
    }
}