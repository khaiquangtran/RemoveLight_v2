#include "LCD16x2.h"

LCD16x2::LCD16x2(RemoteLight *rml) : mRML(rml)
{
	LOGI("Initialization LCD!");
	mRetry = 0U;

retry:
	if (checkAddress())
	{
		mLCD = new LiquidCrystal_I2C(LCD_ADDR, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
		mLCD->init();
		mLCD->backlight();
		DAY[0] = new char[2];
		DAY[0][0] = ' ';
		DAY[0][1] = '\0';
		DAY[1] = new char[4];
		DAY[1][0] = 'S';
		DAY[1][1] = 'U';
		DAY[1][2] = 'N';
		DAY[1][3] = '\0';
		DAY[2] = new char[4];
		DAY[2][0] = 'M';
		DAY[2][1] = 'O';
		DAY[2][2] = 'N';
		DAY[2][3] = '\0';
		DAY[3] = new char[4];
		DAY[3][0] = 'T';
		DAY[3][1] = 'U';
		DAY[3][2] = 'E';
		DAY[3][3] = '\0';
		DAY[4] = new char[4];
		DAY[4][0] = 'W';
		DAY[4][1] = 'E';
		DAY[4][2] = 'D';
		DAY[4][3] = '\0';
		DAY[5] = new char[4];
		DAY[5][0] = 'T';
		DAY[5][1] = 'H';
		DAY[5][2] = 'U';
		DAY[5][3] = '\0';
		DAY[6] = new char[4];
		DAY[6][0] = 'F';
		DAY[6][1] = 'R';
		DAY[6][2] = 'I';
		DAY[6][3] = '\0';
		DAY[7] = new char[4];
		DAY[7][0] = 'S';
		DAY[7][1] = 'A';
		DAY[7][2] = 'T';
		DAY[7][3] = '\0';
	}
	else
	{
		if (mRetry < RETRY)
		{
			mRetry++;
			LOGW("retry %d", mRetry);
			goto retry;
		}
	}
}

LCD16x2::~LCD16x2()
{
	delete mLCD;
	delete[] DAY[0];
	delete[] DAY[1];
	delete[] DAY[2];
	delete[] DAY[3];
	delete[] DAY[4];
	delete[] DAY[5];
	delete[] DAY[6];
	delete[] DAY[7];
}

void LCD16x2::handleSignal(const SignaLType signal, Package *data)
{
	if (mRetry >= RETRY)
	{
		LOGE("Can't connect to LCD16x2");
		return;
	}
	else
	{
		// LOGD("Handle signal value: %d", signal);
		switch (signal)
		{
		case (SignaLType::LCD_DISPLAY_ALL_TIME):
		{
			int *value = data->getPackage();
			mReceiverTime.second = static_cast<uint8_t>(*value++);
			mReceiverTime.minute = static_cast<uint8_t>(*value++);
			mReceiverTime.hour = static_cast<uint8_t>(*value++);
			mReceiverTime.day = static_cast<uint8_t>(*value++);
			mReceiverTime.date = static_cast<uint8_t>(*value++);
			mReceiverTime.month = static_cast<uint8_t>(*value++);
			mReceiverTime.year = static_cast<uint16_t>(*value);
			displayTimeFromDS1307(mReceiverTime);
			break;
		}
		case (SignaLType::LCD_CLEAR_TURN_OFF_SCREEN):
		{
			mLCD->clear();
			mLCD->noBacklight();
			break;
		}
		case (SignaLType::LCD_CLEAR_SCREEN):
		{
			mLCD->clear();
			break;
		}
		case (SignaLType::IR_BTN_APP_SIGNAL):
		{
			displayStartSetupMode();
			break;
		}
		case (SignaLType::LCD_TURN_ON_LIGHT):
		{
			mLCD->backlight();
			break;
		}
		case (SignaLType::LCD_DISPLAY_END_SETUP_MODE):
		{
			displayEndSetupMode();
			break;
		}
		case (SignaLType::IR_BTN_MENU_SIGNAL):
		{
			displayMenuMode(0);
			break;
		}
		case (SignaLType::LCD_MENU_MODE_BACK):
		case (SignaLType::LCD_MOVE_RIGHT_MENU_MODE):
		case (SignaLType::LCD_MOVE_LEFT_MENU_MODE):
		{
			int *value = data->getPackage();
			displayMenuMode(*value);
			break;
		}
		case (SignaLType::LCD_MENU_MODE_OK):
		{
			int *value = data->getPackage();
			displaySelectedMenuMode(value);
			break;
		}
		case (SignaLType::LCD_DISPLAY_START_CONNECT_WIFI):
		{
			displayStartConnectWifi();
			break;
		}
		case (SignaLType::LCD_DISPLAY_CONNECT_WIFI):
		{
			displayConnectingWifi();
			break;
		}
		case (SignaLType::LCD_CONNECT_WIFI_SUCCESS):
		{
			displayConnectWifiSuccess();
			break;
		}
		case (SignaLType::LCD_CONNECT_WIFI_FAILED):
		{
			displayConnectWifiFailed();
			break;
		}
		case (SignaLType::LCD_DISPLAY_START_CONNECT_FIREBASE):
		{
			displayStartConnectFirebase();
			break;
		}
		case (SignaLType::LCD_DISPLAY_START_CONNECT_NTP):
		{
			displayStartConnectNTP();
			break;
		}
		case (SignaLType::LCD_CONNECT_FIREBASE_SUCCESS):
		{
			displayConnectFirebaseSuccess();
			break;
		}
		case (SignaLType::LCD_CONNECT_NTP_SUCCESS):
		{
			displayConnectNTPSuccess();
			break;
		}
		case (SignaLType::LCD_CONNECT_FIREBASE_FAILED):
		{
			displayConnectFBFailed();
			break;
		}
		case (SignaLType::LCD_CONNECT_NTP_FAILED):
		{
			displayConnectNTPFailed();
			break;
		}
		default:
		{
			LOGW("Signal is not supported yet.");
			break;
		}
		}
	}
}

bool LCD16x2::checkAddress()
{
	if (scanAddress(LCD_ADDR) == Hardware::INVALID)
	{
		LOGI("No find LCD");
		return false;
	}
	else
	{
		LOGI("Find out LCD");
		return true;
	}
}

void LCD16x2::displayTimeFromDS1307(struct TimeDS1307 data)
{
	mLCD->setCursor(0, 0);
	mLCD->print(data.day[DAY]);
	mLCD->print(" ");
	if (data.date < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(data.date);
	mLCD->print("/");
	if (data.month < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(data.month);
	mLCD->print("/");
	mLCD->print(data.year);
	mLCD->print("   ");

	mLCD->setCursor(0, 1);
	mLCD->print("Time: ");
	if (data.hour < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(data.hour);
	mLCD->print(":");
	if (data.minute < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(data.minute);
	mLCD->print(":");
	if (data.second < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(data.second);
	mLCD->print("   ");
}

void LCD16x2::displayStartSetupMode()
{
	mLCD->clear();
	mLCD->backlight();
	mLCD->setCursor(0, 0);
	mLCD->print("    CAI DAT     ");
	mLCD->setCursor(0, 1);
	mLCD->print("   THOI GIAN    ");
}

void LCD16x2::displayEndSetupMode()
{
	mLCD->clear();
	mLCD->backlight();
	mLCD->setCursor(0, 0);
	mLCD->print("    CAI DAT     ");
	mLCD->setCursor(0, 1);
	mLCD->print("   HOAN THANH   ");
}

void LCD16x2::displayMenuMode(uint8_t light)
{
	mLCD->setCursor(0, 0);
	mLCD->print("CHON DEN CAI DAT:");
	mLCD->setCursor(0, 1);
	switch (light)
	{
	case 0:
	{
		mLCD->print("  >1< 2  3  4   ");
		break;
	}
	case 1:
	{
		mLCD->print("   1 >2< 3  4   ");
		break;
	}
	case 2:
	{
		mLCD->print("   1  2 >3< 4   ");
		break;
	}
	case 3:
	{
		mLCD->print("   1  2  3 >4<  ");
		break;
	}
	default:
		break;
	}
}

void LCD16x2::displaySelectedMenuMode(int *data)
{
	int flagChooseLight = *data++;
	mLCD->setCursor(0, 0);
	mLCD->print("ON ");
	mLCD->print(flagChooseLight + 1);
	if (*data++)
	{
		mLCD->print(" MO ");
	}
	else
	{
		mLCD->print(" TAT");
	}

	if (*data < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(*data++);

	mLCD->setCursor(0, 1);
	mLCD->print("OFF");
	mLCD->print(flagChooseLight + 1);
	if (*data++)
	{
		mLCD->print(" MO ");
	}
	else
	{
		mLCD->print(" TAT");
	}
	if (*data < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10)
	{
		mLCD->print("0");
	}
	mLCD->print(*data++);
}

void LCD16x2::displayStartConnectWifi()
{
	mLCD->setCursor(0,0);
	mLCD->print("Connecting Wifi  ");
	mLCD->setCursor(0,1);
}

void LCD16x2::displayStartConnectFirebase()
{
	mLCD->setCursor(0,0);
	mLCD->print("Connecting FB    ");
	mLCD->setCursor(0,1);
}

void LCD16x2::displayStartConnectNTP()
{
	mLCD->setCursor(0,0);
	mLCD->print("Connecting NTP  ");
	mLCD->setCursor(0,1);
}

void LCD16x2::displayConnectingWifi()
{
	mLCD->print(".");
}

void LCD16x2::displayConnectWifiSuccess()
{
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect Wifi  ");
	mLCD->setCursor(0,1);
	mLCD->print("Success");
}

void LCD16x2::displayConnectWifiFailed()
{
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect Wifi  ");
	mLCD->setCursor(0,1);
	mLCD->print("Failed");
}

void LCD16x2::displayConnectFirebaseSuccess()
{
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect FB    ");
	mLCD->setCursor(0,1);
	mLCD->print("Success");
}

void LCD16x2::displayConnectNTPSuccess()
{
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect NTP   ");
	mLCD->setCursor(0,1);
	mLCD->print("Success");
}

void LCD16x2::displayConnectFBFailed()
{
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect FB    ");
	mLCD->setCursor(0,1);
	mLCD->print("Failed");
}

void LCD16x2::displayConnectNTPFailed()
{
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect NTP   ");
	mLCD->setCursor(0,1);
	mLCD->print("Failed");
}