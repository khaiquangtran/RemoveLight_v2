#include "./Tasks.h"

Tasks::Tasks(std::shared_ptr<RemoteLight> rml, std::shared_ptr<Hardware> lcd, std::shared_ptr<Hardware> rtc, std::shared_ptr<Hardware> ir, std::shared_ptr<Network> net) :
    mRML(rml), mLCD(lcd), mNET(net), mRTC(rtc), mIR(ir)
{
    mCounterConnectWifi = 0U;
    mCounterDisplayAllTime = 0U;
    mFlagConnectFirebase = CONNECT_STATUS::UNKNOWN;
    mModeHandle = MODE_HANDLE::NONE;

    mPrintListModeHandle = {
        {MODE_HANDLE::NONE,                 "MODE_HANDLE::NONE"},
        {MODE_HANDLE::INTO_SETUP_MODE,      "MODE_HANDLE::INTO_SETUP_MODE"},
        {MODE_HANDLE::INTO_MENU_MODE,       "MODE_HANDLE::INTO_MENU_MODE"},
        {MODE_HANDLE::MENU_MODE,            "MODE_HANDLE::MENU_MODE"},
    };
}

void Tasks::handleSignal(const SignalType signal, Package *data)
{
    switch (signal) {
    case (SignalType::TASKS_CONNECT_WIFI): {
        connectWifiMode();
        break;
    }
    case (SignalType::TASKS_CONNECT_WIFI_SUCCESS): {
        mCounterConnectWifi = 0;
        LOGI("WIFI connection SUCCESS!");
        mLCD->handleSignal(SignalType::LCD_CONNECT_WIFI_SUCCESS);
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
        mRML->handleSignal(SignalType::REMOTE_LIGHT_UPDATE_TIMER_CONNECT_FIREBASE);
        break;
    }
    case (SignalType::TASKS_CONNECT_FIREBASE): {
        connectFirebaseMode();
        break;
    }
    case (SignalType::TASKS_CONNECT_FIREBASE_SUCCESS): {
        mCounterConnectWifi = 0;
        mFlagConnectFirebase = CONNECT_STATUS::SUCCESS;
        LOGI("FIREBASE connection SUCCESS!");
        mLCD->handleSignal(SignalType::LCD_CONNECT_FIREBASE_SUCCESS);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_REMOVE_CONNECT_FIREBASE_MODE);
        break;
    }
    case (SignalType::TASKS_CONNECT_FIREBASE_FAILED): {
        LOGW("FIREBASE connection FAILED!");
        mFlagConnectFirebase = CONNECT_STATUS::FAILED;
        break;
    }
    case (SignalType::TIMER_CONNECT_FIREBASE_SIGNAL): {
        connectFirebaseTimeout();
        break;
    }
    case (SignalType::TIMER_CONNECT_FIREBASE_SUCCESS_GOTO_NEXT_CONNECT):
    case (SignalType::TIMER_CONNECT_FIREBASE_FAILED_GOTO_NEXT_CONNECT): {
        mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_NTP);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_NTP);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_UPDATE_TIMER_CONNECT_NTP);
        break;
    }
    case (SignalType::TASKS_CONNECT_NTP): {
        connectNTPMode();
        break;
    }
    case (SignalType::TASKS_CONNECT_NTP_SUCCESS): {
        mCounterConnectWifi = 0;
        LOGI("NTP connection SUCCESS!");
        // mRTC->handleSignal(SignalType::RTC_SET_FLAG_UPDATE_TIME_WITH_NTP_SUCCESS);
        mLCD->handleSignal(SignalType::LCD_CONNECT_NTP_SUCCESS);
        mNET->handleSignal(SignalType::NETWORK_GET_TIME_DATE_FROM_NTP);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_REMOVE_CONNECT_NTP_MODE);
        break;
    }
    case (SignalType::TASKS_CONNECT_NTP_FAILED): {
        LOGW("NTP connection FAILED!");
        break;
    }
    case (SignalType::TIMER_CONNECT_NTP_SIGNAL): {
        connectNTPTimeout();
        break;
    }
    case (SignalType::TIMER_CONNECT_WIFI_FAILED_GOTO_NEXT_MODE):
    case (SignalType::TIMER_CONNECT_NTP_SUCCESS_GOTO_NEXT_CONNECT):
    case (SignalType::TIMER_CONNECT_NTP_FAILED_GOTO_NEXT_CONNECT):
    case (SignalType::TIMER_DISPLAY_ALL_END_SETUP_MODE_SIGNAL): {
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
    case (SignalType::IR_BTN_APP_SIGNAL): {
        mRML->handleSignal(SignalType::REMOTE_LIGHT_STOP_TIMER_CHECK_CONFIGURED_TIMER_FOR_LIGHT);
        displayReadySetupMode();
        break;
    }
    case (SignalType::TASKS_START_SETUP_MODE): {
        if(mModeHandle == MODE_HANDLE::NONE) {
            intoSetupMode();
        }
        else {
            LOGW("Can't \'TASK DISPLAY ALL TIME\'. During %s!", mPrintListModeHandle.at(mModeHandle).c_str());
        }
        break;
    }
    case (SignalType::TASKS_END_SETUP_MODE): {
        displayEndSetupMode();
        break;
    }
    case (SignalType::IR_BTN_UP_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			// mRTC->handleSignal(SignalType::RTC_INCREASE_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			// mRTC->handleSignal(SignalType::RTC_INCREASE_VALUE_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_DOWN_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			// mRTC->handleSignal(SignalType::RTC_DECREASE_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			// mRTC->handleSignal(SignalType::RTC_DECREASE_VALUE_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_LEFT_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			// mRTC->handleSignal(SignalType::RTC_SHIFT_LEFT_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
			// mRTC->handleSignal(SignalType::RTC_MOVE_LEFT_MENU_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			// mRTC->handleSignal(SignalType::RTC_MOVE_LEFT_INTO_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_RIGHT_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			// mRTC->handleSignal(SignalType::RTC_SHIFT_RIGHT_VALUE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
			// mRTC->handleSignal(SignalType::RTC_MOVE_RIGHT_MENU_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
			// mRTC->handleSignal(SignalType::RTC_MOVE_RIGHT_INTO_MENU_MODE);
		}
		break;
	}
    case (SignalType::IR_BTN_OK_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::INTO_SETUP_MODE) {
			// mRTC->handleSignal(SignalType::RTC_SETUP_MODE_OK);
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			mLCD->handleSignal(SignalType::LCD_DISPLAY_END_SETUP_MODE);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_END_SETUP_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
            mModeHandle = MODE_HANDLE::INTO_MENU_MODE;
            mRML->handleSignal(SignalType::REMOTE_LIGHT_INTO_MENU_MODE);
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			// mRTC->handleSignal(SignalType::RTC_MENU_MODE_OK);
		}
		break;
	}
    case (SignalType::IR_BTN_MENU_SIGNAL): {
		if (mModeHandle == MODE_HANDLE::NONE) {
            LOGI("Start MENU mode!");
            mModeHandle = MODE_HANDLE::MENU_MODE;
            if(mCounterDisplayAllTime != 0){
                mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_DISPLAY_ALLLTIME_STOP);
            }
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
			mLCD->handleSignal(SignalType::IR_BTN_MENU_SIGNAL);
            // mRML->handleSignal(SignalType::REMOTE_LIGHT_MENU_MODE);
		}
        else {
            LOGW("Can't \'TASK MENU MODE\'. During %s!", mPrintListModeHandle.at(mModeHandle).c_str());
        }
		break;
	}
    case (SignalType::IR_BTN_BACK_SIGNAL): {
        if (mModeHandle == MODE_HANDLE::INTO_MENU_MODE) {
            mModeHandle = MODE_HANDLE::MENU_MODE;
			mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
			// mRTC->handleSignal(SignalType::RTC_BACK_MENU_MODE);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_MENU_MODE);
		}
		else if (mModeHandle == MODE_HANDLE::MENU_MODE) {
            LOGI("End MENU mode!");
            mModeHandle = MODE_HANDLE::NONE;
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
    case (SignalType::IR_BTN_5_SIGNAL): {
        if(mCounterDisplayAllTime == 0) {
            mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
        }
        else {
            LOGW("DISPLAY ALL TIME is in progress");
        }
        break;
    }
    case (SignalType::IR_BTN_6_SIGNAL): {
        if(mCounterDisplayAllTime == 0) {
            mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
            mRML->handleSignal(SignalType::REMOTE_UPDATE_TIMER_DISPLAY_ALLTIME_TO_TEMPPRESSS);
            mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_TEMP_PRESS);
        }
        else {
            LOGW("DISPLAY TEMP PRESS is in progress");
        }
        break;
    }
    case (SignalType::TIMER_DISPLAY_TEMP_PRESS_SIGNAL): {
        displayTempPressTimeout();
        break;
    }
    case (SignalType::TASKS_STOP_DISPLAY_ALL_TIME): {
        mCounterDisplayAllTime = 0;
        mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
        break;
    }
    case (SignalType::TASKS_WIFI_PROVISIONING_START): {
        mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
        break;
    }
    default: break;
    }
}

void Tasks::connectWifiMode() {
    mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_CONNECT_WIFI_START);
    mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_WIFI);
    mLCD->handleSignal(SignalType::LCD_DISPLAY_CONNECT_WIFI);
    LOGI("WIFI connection %d times", mCounterConnectWifi + 1);
	mNET->handleSignal(SignalType::NETWORK_CHECK_STATUS_WIFI);
}

void Tasks::connectFirebaseMode() {
    mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_FIREBASE);
    mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_FIREBASE);
    mLCD->handleSignal(SignalType::LCD_DISPLAY_CONNECT_WIFI);
    LOGI("FIREBASE connection %d times", mCounterConnectWifi + 1);
    mNET->handleSignal(SignalType::NETWORK_CHECK_STATUS_FIREBASE);
}

void Tasks::connectNTPMode() {
    mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_NTP);
    mLCD->handleSignal(SignalType::LCD_DISPLAY_START_CONNECT_NTP);
    mLCD->handleSignal(SignalType::LCD_DISPLAY_CONNECT_WIFI);
    LOGI("NTP connection %d times", mCounterConnectWifi + 1);
    mNET->handleSignal(SignalType::NETWORK_CHECK_STATUS_NTP);
}

void Tasks::connectWifiTimeout() {
    if (mCounterConnectWifi < (REPEATS_5 - 1)) {
        mCounterConnectWifi++;
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_WIFI);
    }
    else {
        LOGW("WIFI connection FAILED!");
        mCounterConnectWifi = 0;
        // mRML->handleSignal(SignalType::REMOTE_LIGHT_CONTROL_MODE_NONE);
        mLCD->handleSignal(SignalType::LCD_CONNECT_WIFI_FAILED);
        delay(1000);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_CONNECT_WIFI_TIMEOUT);
    }
}

void Tasks::connectFirebaseTimeout() {
    if (mCounterConnectWifi < (REPEATS_10 - 1)) {
        mCounterConnectWifi++;
        mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_CONNECT_WIFI_START);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_FIREBASE);
    }
    else {
        LOGW("Firebase connection FAILED!");
        mCounterConnectWifi = 0;
        // mRML->handleSignal(SignalType::REMOTE_LIGHT_CONTROL_MODE_NONE);
        mLCD->handleSignal(SignalType::LCD_CONNECT_FIREBASE_FAILED);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_CONNECT_FIREBASE_TIMEOUT);
    }
}

void Tasks::connectNTPTimeout() {
    if (mCounterConnectWifi < (REPEATS_10 - 1)) {
		mCounterConnectWifi++;
        mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_CONNECT_WIFI_START);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_CONNECT_NTP);
	}
	else {
        LOGW("NTP connection FAILED!");
		mCounterConnectWifi = 0;
        // mRML->handleSignal(SignalType::REMOTE_LIGHT_CONTROL_MODE_NONE);
        mRTC->handleSignal(SignalType::RTC_SET_FLAG_UPDATE_TIME_WITH_NTP_FAILED);
		mLCD->handleSignal(SignalType::LCD_CONNECT_NTP_FAILED);
		mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_CONNECT_NTP_TIMEOUT);
	}
}

void Tasks::displayAllTime() {
	mRTC->handleSignal(SignalType::RTC_DISPLAY_ALL_TIME);
	mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_DISPLAY_ALLLTIME_START);
}

void Tasks::displayAllTimeTimeout()
{
    LOGI("mCounterDisplayAllTime: %d", mCounterDisplayAllTime);
    if (mCounterDisplayAllTime < REPEATS_30) {
		mCounterDisplayAllTime++;
        mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_ALLTIME);
	}
	else {
		mCounterDisplayAllTime = 0;
		mLCD->handleSignal(SignalType::LCD_CLEAR_TURN_OFF_SCREEN);
        mRML->handleSignal(SignalType::REMOTE_LIGHT_REMOVE_DISPLAY_ALL_TIME_MODE);
	}
}

void Tasks::displayReadySetupMode()
{
    LOGI(".");
    mLCD->handleSignal(SignalType::IR_BTN_APP_SIGNAL);
    if(mCounterDisplayAllTime != 0) {
        mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_DISPLAY_ALLLTIME_STOP);
    }
	mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_DISPLAY_SETUP_MODE_START);
}

void Tasks::intoSetupMode()
{
    mModeHandle = MODE_HANDLE::INTO_SETUP_MODE;
	// mRTC->handleSignal(SignalType::RTC_DISPLAY_ALL_TIME);
}

void Tasks::displayEndSetupMode()
{
    mModeHandle = MODE_HANDLE::NONE;
	mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_DISPLAY_SETUP_MODE_STOP);
}

void Tasks::installIRButton()
{
    if (mModeHandle == MODE_HANDLE::NONE)
	{
        if(mCounterDisplayAllTime != 0) {
            mRML->handleSignal(SignalType::REMOTE_LIGHT_TIMER_DISPLAY_ALLLTIME_STOP);
        }
		mModeHandle = MODE_HANDLE::INSTALL_IR_BUTTON;
		//mIR->handleSignal(SignalType::IR_INSTALL_BUTTON);
        mLCD->handleSignal(SignalType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignalType::LCD_TURN_ON_LIGHT);
		mLCD->handleSignal(SignalType::LCD_INSTALL_BUTTON1);
	}
	else if (mModeHandle == MODE_HANDLE::INSTALL_IR_BUTTON) {
		LOGI("Install button Done. Move next button");
		//mIR->handleSignal(SignalType::IR_INSTALL_BUTTON_DONE);
	}
    else {
        // Do nothing
    }
}

void Tasks::displayTempPressTimeout()
{
    LOGI("mCounterDisplayAllTime: %d", mCounterDisplayAllTime);
    if (mCounterDisplayAllTime < REPEATS_10) {
		mCounterDisplayAllTime++;
        mRML->handleSignal(SignalType::REMOTE_LIGHT_DISPLAY_TEMP_PRESS);
	}
	else {
		mCounterDisplayAllTime = 0;
        mRML->handleSignal(SignalType::REMOTE_UPDATE_TIMER_DISPLAY_TEMPPRESSS_TO_ALLTIME);
		mLCD->handleSignal(SignalType::LCD_CLEAR_TURN_OFF_SCREEN);
	}
}