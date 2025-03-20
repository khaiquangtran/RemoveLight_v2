#include "LCD16x2.h"

LCD16x2::LCD16x2(std::shared_ptr<RemoteLight> rml) : mRML(rml)
{
	LOGI("Initialization LCD!");
	mRetry = 0U;

	mButtonStringMap = {
		{1, "Button 1"},
		{2, "Button 2"},
		{3, "Button 3"},
		{4, "Button 4"},
		{5, "Button Up"},
		{6, "Button Down"},
		{7, "Button Right"},
		{8, "Button Left"},
		{9, "Button Ok"},
		{10, "Button Menu"},
		{11, "Button App"},
		{12, "Button Back"},
	};

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

void LCD16x2::handleSignal(const SignalType signal, Package *data)
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
		case (SignalType::LCD_DISPLAY_ALL_TIME):
		{
			int *value = data->getPackage();
			mReceiverTime.second 	= static_cast<uint8_t>(*value++);
			mReceiverTime.minute 	= static_cast<uint8_t>(*value++);
			mReceiverTime.hour 		= static_cast<uint8_t>(*value++);
			mReceiverTime.day 		= static_cast<uint8_t>(*value++);
			mReceiverTime.date 		= static_cast<uint8_t>(*value++);
			mReceiverTime.month 	= static_cast<uint8_t>(*value++);
			mReceiverTime.year 		= static_cast<uint16_t>(*value);
			displayTimeFromDS1307(mReceiverTime);
			break;
		}
		case (SignalType::LCD_CLEAR_TURN_OFF_SCREEN):
			mLCD->clear();
			mLCD->noBacklight();
			break;
		case (SignalType::LCD_CLEAR_SCREEN):
			mLCD->clear();
			break;
		case (SignalType::IR_BTN_APP_SIGNAL):
			displayStartSetupMode();
			break;
		case (SignalType::LCD_TURN_ON_LIGHT):
			mLCD->backlight();
			break;
		case (SignalType::LCD_DISPLAY_END_SETUP_MODE):
			displayEndSetupMode();
			break;
		case (SignalType::IR_BTN_MENU_SIGNAL):
			displayMenuMode(0);
			break;
		case (SignalType::LCD_MENU_MODE_BACK):
		case (SignalType::LCD_MOVE_RIGHT_MENU_MODE):
		case (SignalType::LCD_MOVE_LEFT_MENU_MODE):
		{
			int *value = data->getPackage();
			displayMenuMode(*value);
			break;
		}
		case (SignalType::LCD_MENU_MODE_OK):
		{
			int *value = data->getPackage();
			displaySelectedMenuMode(value);
			break;
		}
		case (SignalType::LCD_DISPLAY_START_CONNECT_WIFI):
			displayStartConnectWifi();
			break;
		case (SignalType::LCD_DISPLAY_CONNECT_WIFI):
			displayConnectingWifi();
			break;
		case (SignalType::LCD_CONNECT_WIFI_SUCCESS):
			displayConnectWifiSuccess();
			break;
		case (SignalType::LCD_CONNECT_WIFI_FAILED):
			displayConnectWifiFailed();
			break;
		case (SignalType::LCD_DISPLAY_START_CONNECT_FIREBASE):
			displayStartConnectFirebase();
			break;
		case (SignalType::LCD_DISPLAY_START_CONNECT_NTP):
			displayStartConnectNTP();
			break;
		case (SignalType::LCD_CONNECT_FIREBASE_SUCCESS):
			displayConnectFirebaseSuccess();
			break;
		case (SignalType::LCD_CONNECT_NTP_SUCCESS):
			displayConnectNTPSuccess();
			break;
		case (SignalType::LCD_CONNECT_FIREBASE_FAILED):
			displayConnectFBFailed();
			break;
		case (SignalType::LCD_CONNECT_NTP_FAILED):
			displayConnectNTPFailed();
			break;
		case (SignalType::LCD_INSTALL_BUTTON1):
			mLCD->clear();
			mLCD->setCursor(0, 0);
			mLCD->print("BUTTON1");
			break;
		case (SignalType::REMOTE_LIGHT_IRBUTTON_INSTALL):
			displayInstallButton(data);
			break;
		default:
			LOGW("Signal is not supported yet.");
			break;
		}
	}
}

bool LCD16x2::checkAddress()
{
	if (scanAddress(LCD_ADDR) == Hardware::INVALID) {
		LOGI("No find LCD");
		return false;
	}
	else {
		LOGI("Find out LCD");
		return true;
	}
}

void LCD16x2::displayTimeFromDS1307(struct TimeDS1307 data)
{
	mLCD->setCursor(0, 0);
	mLCD->print(data.day[DAY]);
	mLCD->print(" ");
	if (data.date < 10) {
		mLCD->print("0");
	}
	mLCD->print(data.date);
	mLCD->print("/");
	if (data.month < 10) {
		mLCD->print("0");
	}
	mLCD->print(data.month);
	mLCD->print("/");
	mLCD->print(data.year);
	mLCD->print("   ");

	mLCD->setCursor(0, 1);
	mLCD->print("Time: ");
	if (data.hour < 10) {
		mLCD->print("0");
	}
	mLCD->print(data.hour);
	mLCD->print(":");
	if (data.minute < 10) {
		mLCD->print("0");
	}
	mLCD->print(data.minute);
	mLCD->print(":");
	if (data.second < 10) {
		mLCD->print("0");
	}
	mLCD->print(data.second);
	mLCD->print("   ");
}

void LCD16x2::displayStartSetupMode()
{
	LOGI(".");
	mLCD->clear();
	mLCD->backlight();
	mLCD->setCursor(0, 0);
	mLCD->print("    CAI DAT     ");
	mLCD->setCursor(0, 1);
	mLCD->print("   THOI GIAN    ");
}

void LCD16x2::displayEndSetupMode()
{
	LOGI(".");
	mLCD->clear();
	mLCD->backlight();
	mLCD->setCursor(0, 0);
	mLCD->print("    CAI DAT     ");
	mLCD->setCursor(0, 1);
	mLCD->print("   HOAN THANH   ");
}

void LCD16x2::displayMenuMode(uint8_t light)
{
	LOGI(".");
	mLCD->setCursor(0, 0);
	mLCD->print("CHON DEN CAI DAT:");
	mLCD->setCursor(0, 1);
	switch (light)
	{
	case 0:
		mLCD->print("  >1< 2  3  4   ");
		break;
	case 1:
		mLCD->print("   1 >2< 3  4   ");
		break;
	case 2:
		mLCD->print("   1  2 >3< 4   ");
		break;
	case 3:
		mLCD->print("   1  2  3 >4<  ");
		break;
	default:
		break;
	}
}

void LCD16x2::displaySelectedMenuMode(int *data)
{
	LOGI(".");
	int flagChooseLight = *data++;
	mLCD->setCursor(0, 0);
	mLCD->print("ON ");
	mLCD->print(flagChooseLight + 1);
	if (*data++) {
		mLCD->print(" MO ");
	}
	else {
		mLCD->print(" TAT");
	}

	if (*data < 10) {
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10) {
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10) {
		mLCD->print("0");
	}
	mLCD->print(*data++);

	mLCD->setCursor(0, 1);
	mLCD->print("OFF");
	mLCD->print(flagChooseLight + 1);
	if (*data++) {
		mLCD->print(" MO ");
	}
	else {
		mLCD->print(" TAT");
	}
	if (*data < 10) {
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10) {
		mLCD->print("0");
	}
	mLCD->print(*data++);
	mLCD->print(":");
	if (*data < 10) {
		mLCD->print("0");
	}
	mLCD->print(*data++);
}

void LCD16x2::displayStartConnectWifi()
{
	LOGI("Connecting Wifi");
	mLCD->setCursor(0,0);
	mLCD->print("Connecting Wifi  ");
	mLCD->setCursor(0,1);
}

void LCD16x2::displayStartConnectFirebase()
{
	LOGI("Connecting FB");
	mLCD->setCursor(0,0);
	mLCD->print("Connecting FB    ");
	mLCD->setCursor(0,1);
}

void LCD16x2::displayStartConnectNTP()
{
	LOGI("Connecting NTP");
	mLCD->setCursor(0,0);
	mLCD->print("Connecting NTP  ");
	mLCD->setCursor(0,1);
}

void LCD16x2::displayConnectingWifi()
{
	LOGI(".");
	mLCD->print(".");
}

void LCD16x2::displayConnectWifiSuccess()
{
	LOGI("Connect Wifi Success");
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect Wifi  ");
	mLCD->setCursor(0,1);
	mLCD->print("Success");
}

void LCD16x2::displayConnectWifiFailed()
{
	LOGI("Connect Wifi Failed");
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect Wifi  ");
	mLCD->setCursor(0,1);
	mLCD->print("Failed");
}

void LCD16x2::displayConnectFirebaseSuccess()
{
	LOGI("Connect FB Success");
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect FB    ");
	mLCD->setCursor(0,1);
	mLCD->print("Success");
}

void LCD16x2::displayConnectNTPSuccess()
{
	LOGI("Connect NTP Success");
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect NTP   ");
	mLCD->setCursor(0,1);
	mLCD->print("Success");
}

void LCD16x2::displayConnectFBFailed()
{
	LOGI("Connect FB Failed");
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect FB    ");
	mLCD->setCursor(0,1);
	mLCD->print("Failed");
}

void LCD16x2::displayConnectNTPFailed()
{
	LOGI("Connect NTP Failed");
	mLCD->clear();
	mLCD->setCursor(0,0);
	mLCD->print("Connect NTP   ");
	mLCD->setCursor(0,1);
	mLCD->print("Failed");
}

void LCD16x2::displayInstallButton(Package *data)
{
	if(data->getSize() == 3)
	{
		int *parseData = data->getPackage();
		String str;
		if(mButtonStringMap.find(parseData[0]) != mButtonStringMap.end()) {
			str = mButtonStringMap.at(parseData[0]);
		}
		int total =  (parseData[1] << 16) | parseData[2];
		LOGD("Total: %x", total);
		mLCD->clear();
		mLCD->setCursor(0,0);
		mLCD->print(str);
		mLCD->setCursor(0,1);
		mLCD->print("0x");
		mLCD->print(total, HEX);
	}
	else if(data->getSize() == 1) {
		int *parseData = data->getPackage();
		String str;
		if(mButtonStringMap.find(parseData[0]) != mButtonStringMap.end()) {
			str = mButtonStringMap.at(parseData[0]);
		}
		mLCD->clear();
		mLCD->setCursor(0,0);
		mLCD->print(str);
		mLCD->setCursor(0,1);
		mLCD->print("DONE");
	}
	else {
		// Do nothing
	}
}