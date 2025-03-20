#include "RTC.h"

RTC::RTC(std::shared_ptr<RemoteLight> rml) : mRML(rml), mRTCAddr(0U), mIndexOfAllTimeData(0U), mIndexLight(0U),
								mIndexListLight(0U), mCountRetry(0U), mFlagUpdateTIme(0U), mPreDate(0U),
								mCounterUpdateTime(0U), mCounterInstallIRButton(0U)
{
	Wire.begin();

	mTimeOfLight["Light1"] = std::make_pair(
		std::make_pair(REG_TIME_LIGHT{LIGHT1::ON_SWITCH, LIGHT1::ON_HOUR, LIGHT1::ON_MINUTE, LIGHT1::ON_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}),
		std::make_pair(REG_TIME_LIGHT{LIGHT1::OFF_SWITCH, LIGHT1::OFF_HOUR, LIGHT1::OFF_MINUTE, LIGHT1::OFF_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}));
	mTimeOfLight["Light2"] = std::make_pair(
		std::make_pair(REG_TIME_LIGHT{LIGHT2::ON_SWITCH, LIGHT2::ON_HOUR, LIGHT2::ON_MINUTE, LIGHT2::ON_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}),
		std::make_pair(REG_TIME_LIGHT{LIGHT2::OFF_SWITCH, LIGHT2::OFF_HOUR, LIGHT2::OFF_MINUTE, LIGHT2::OFF_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}));
	mTimeOfLight["Light3"] = std::make_pair(
		std::make_pair(REG_TIME_LIGHT{LIGHT3::ON_SWITCH, LIGHT3::ON_HOUR, LIGHT3::ON_MINUTE, LIGHT3::ON_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}),
		std::make_pair(REG_TIME_LIGHT{LIGHT3::OFF_SWITCH, LIGHT3::OFF_HOUR, LIGHT3::OFF_MINUTE, LIGHT3::OFF_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}));
	mTimeOfLight["Light4"] = std::make_pair(
		std::make_pair(REG_TIME_LIGHT{LIGHT4::ON_SWITCH, LIGHT4::ON_HOUR, LIGHT4::ON_MINUTE, LIGHT4::ON_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}),
		std::make_pair(REG_TIME_LIGHT{LIGHT4::OFF_SWITCH, LIGHT4::OFF_HOUR, LIGHT4::OFF_MINUTE, LIGHT4::OFF_SECOND}, TimeOfLight{0U, 0U, 0U, 0U}));

	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT1_DATA_REQUEST] = std::make_pair(0, SignalType::WEB_GET_LIGHT1_DATA_RESPONSE);
	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT2_DATA_REQUEST] = std::make_pair(1, SignalType::WEB_GET_LIGHT2_DATA_RESPONSE);
	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT3_DATA_REQUEST] = std::make_pair(2, SignalType::WEB_GET_LIGHT3_DATA_RESPONSE);
	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT4_DATA_REQUEST] = std::make_pair(3, SignalType::WEB_GET_LIGHT4_DATA_RESPONSE);

	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT1_DATA_REQUEST] = std::make_pair(0, SignalType::WEB_SET_LIGHT1_DATA_RESPONSE);
	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT2_DATA_REQUEST] = std::make_pair(1, SignalType::WEB_SET_LIGHT2_DATA_RESPONSE);
	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT3_DATA_REQUEST] = std::make_pair(2, SignalType::WEB_SET_LIGHT3_DATA_RESPONSE);
	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT4_DATA_REQUEST] = std::make_pair(3, SignalType::WEB_SET_LIGHT4_DATA_RESPONSE);

	mAllTimeData = {0U, 0U, 0U, 0U, 0U, 0U, 0U};

retry:
	if (checkAddress())
	{
		std::map<String, LightMapValue>::iterator it;
		for (it = mTimeOfLight.begin(); it != mTimeOfLight.end(); it++)
		{
			it->second.first.second = getTimeLight(it->first, it->second.first.first.SWITCH);
			it->second.second.second = getTimeLight(it->first, it->second.second.first.SWITCH);
		}
	}
	else
	{
		if (mCountRetry < RETRY)
		{
			mCountRetry++;
			LOGW("retry %d", mCountRetry);
			goto retry;
		}
	}

	LOGI("Initialization RTC!");
}

RTC::~RTC()
{
}

void RTC::handleSignal(const SignalType signal, Package *data)
{
	if (mCountRetry >= RETRY){
		LOGE("Can't connect to RTC");
		return;
	}
	else {
		LOGD("Handle signal value: %d", signal);
		switch (signal)
		{
		case SignalType::RTC_DISPLAY_ALL_TIME:
			mAllTimeData = getTimeData();
			sendAllTimeData(SignalType::LCD_DISPLAY_ALL_TIME);
			break;
		case SignalType::RTC_INCREASE_VALUE:
			increaseValueOfTimeData();
			sendAllTimeData(SignalType::LCD_DISPLAY_ALL_TIME);
			break;
		case SignalType::RTC_DECREASE_VALUE:
			decreaseValueOfTimeData();
			sendAllTimeData(SignalType::LCD_DISPLAY_ALL_TIME);
			break;
		case SignalType::RTC_SHIFT_LEFT_VALUE:
			shiftIndexOfAllTimeData(true);
			break;
		case SignalType::RTC_SHIFT_RIGHT_VALUE:
			shiftIndexOfAllTimeData(false);
			break;
		case SignalType::RTC_SETUP_MODE_OK:
			setTimeData(mAllTimeData);
			break;
		case SignalType::RTC_MOVE_LEFT_MENU_MODE:
		{
			if (mIndexListLight > 0U) {
				mIndexListLight--;
			}
			else if (mIndexListLight == 0U) {
				mIndexListLight = 3U;
			}
			int value = static_cast<int>(mIndexListLight);
			Package data(&value, 1);
			mRML->handleSignal(SignalType::LCD_MOVE_LEFT_MENU_MODE, &data);
			break;
		}
		case SignalType::RTC_MOVE_RIGHT_MENU_MODE:
		{
			if (mIndexListLight < 3U) {
				mIndexListLight++;
			}
			else if (mIndexListLight >= 3U) {
				mIndexListLight = 0U;
			}
			int value = static_cast<int>(mIndexListLight);
			Package data(&value, 1);
			mRML->handleSignal(SignalType::LCD_MOVE_RIGHT_MENU_MODE, &data);
			break;
		}
		case SignalType::RTC_MENU_MODE_OK:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second = getTimeOfLight(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.first.SWITCH);
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second = getTimeOfLight(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.first.SWITCH);
			mIndexLight = 0U;
			sendTimeOfLight();
			break;
		case SignalType::RTC_INCREASE_VALUE_MENU_MODE:
			increaseValueOfMenuMode();
			sendTimeOfLight();
			break;
		case SignalType::RTC_DECREASE_VALUE_MENU_MODE:
			decreaseValueOfMenuMode();
			sendTimeOfLight();
			break;
		case SignalType::RTC_MOVE_RIGHT_INTO_MENU_MODE:
			if (mIndexLight < 7U) {
				mIndexLight++;
			}
			else if (mIndexLight >= 7U) {
				mIndexLight = 0U;
			}
			break;
		case SignalType::RTC_MOVE_LEFT_INTO_MENU_MODE:
			if (mIndexLight > 0U) {
				mIndexLight--;
			}
			else if (mIndexLight == 0U) {
				mIndexLight = 7U;
			}
			break;
		case SignalType::RTC_BACK_MENU_MODE:
		{
			// store data
			setTimeLight(LISTLIGHT[mIndexListLight], mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second, mTimeOfLight[LISTLIGHT[mIndexListLight]].first.first);
			setTimeLight(LISTLIGHT[mIndexListLight], mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second, mTimeOfLight[LISTLIGHT[mIndexListLight]].second.first);
			int value = static_cast<int>(mIndexListLight);
			Package data(&value, 1);
			mRML->handleSignal(SignalType::LCD_MENU_MODE_BACK, &data);
			break;
		}
		case SignalType::WEB_GET_ALLTIME_DATA_REQUEST:
			sendAllTimeDataToWeb();
			break;
		case SignalType::RTC_CHECK_CONFIGURED_TIME_FOR_LIGHT:
			checkConfigureTimeForLight();
			updateTimeForRTC();
			break;
		case SignalType::WEB_SET_ALLTIME_DATA_REQUEST:
			requestSetTimeAllData(data);
			break;
		case SignalType::WEB_GET_LIGHT1_DATA_REQUEST:
		case SignalType::WEB_GET_LIGHT2_DATA_REQUEST:
		case SignalType::WEB_GET_LIGHT3_DATA_REQUEST:
		case SignalType::WEB_GET_LIGHT4_DATA_REQUEST:
			sendLightDataToWeb(signal);
			break;
		case SignalType::WEB_SET_LIGHT1_DATA_REQUEST:
		case SignalType::WEB_SET_LIGHT2_DATA_REQUEST:
		case SignalType::WEB_SET_LIGHT3_DATA_REQUEST:
		case SignalType::WEB_SET_LIGHT4_DATA_REQUEST:
			requestSetLightData(data, signal);
			break;
		case SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP:
			receiveTimeDateFromNTP(data);
			break;
		case SignalType::RTC_GET_ALL_ALL:
		{
			mAllTimeData = getTimeData();
			mPreDate = mAllTimeData.date;
			break;
		}
		case SignalType::RTC_SET_FLAG_UPDATE_TIME_WITH_NTP_SUCCESS:
		{
			mFlagUpdateTIme = 0U;
			break;
		}
		case SignalType::RTC_SET_FLAG_UPDATE_TIME_WITH_NTP_FAILED:
		{
			mFlagUpdateTIme = 3U;
			mCounterUpdateTime = 0U;
			break;
		}
		case SignalType::RTC_COUNTER_INSTALL_IRBUTTON:
		{
			mCounterInstallIRButton = 1;
			break;
		}
		case SignalType::REMOTE_LIGHT_PRESS_BUTTON_REACHED_MAX_TIME:
		{
			mCounterInstallIRButton = 0;
			break;
		}
		default:
			LOGW("Signal is not supported yet.");
			break;
		}
	}
}

bool RTC::checkAddress()
{
	if (scanAddress(DS1307_ADDR) == Hardware::INVALID) {
		LOGI("No find DS1307");
		return false;
	}
	else {
		mRTCAddr = DS1307_ADDR;
		LOGI("Find out DS1307");
		return true;
	}
}

byte RTC::bcdToDec(byte val)
{
	return ((val / 16 * 10) + (val % 16));
};

uint8_t RTC::decToHex(uint8_t val)
{
	return ((val / 10 * 16) + (val % 10));
};

struct TimeDS1307 RTC::getTimeData()
{
	struct TimeDS1307 data{
		0U, 0U, 0U, 0U, 0U, 0U, 0U};
	if (mRTCAddr == 0) {
		return data;
	}
	else {
		Wire.beginTransmission(mRTCAddr);
		Wire.write(0x00);
		Wire.endTransmission();
		if (Wire.requestFrom(mRTCAddr, 7U) == 7U) {
			data.second = bcdToDec(Wire.read() & 0x7f);
			data.minute = bcdToDec(Wire.read());
			data.hour = bcdToDec(Wire.read() & 0x3f);
			data.day = bcdToDec(Wire.read());
			data.date = bcdToDec(Wire.read());
			data.month = bcdToDec(Wire.read());
			data.year = bcdToDec(Wire.read()) + 2000;
			LOGI("%d/%d/%d %d %d:%d:%d", data.date, data.month, data.year, data.day, data.hour, data.minute, data.second);
			return data;
		}
		else {
			LOGE("Failed to get RTC date and time");
			return data;
		};
	}
}

struct TimeOfLight RTC::getTimeOfLight(uint8_t reg)
{
	struct TimeOfLight data{
		0U, 0U, 0U, 0U};
	if (mRTCAddr == 0) {
		return data;
	}
	else {
		Wire.beginTransmission(mRTCAddr);
		Wire.write(reg);
		Wire.endTransmission();
		if (Wire.requestFrom(mRTCAddr, 4U) == 4U) {
			data.sw = bcdToDec(Wire.read() & 0x7f);
			data.hour = bcdToDec(Wire.read());
			data.minute = bcdToDec(Wire.read() & 0x3f);
			data.second = bcdToDec(Wire.read());
			LOGI("%d %d:%d:%d", data.sw, data.hour, data.minute, data.second);
			return data;
		}
		else {
			LOGE("Failed to get RTC date and time");
			return data;
		};
	}
}

bool RTC::writeData(uint8_t reg, uint8_t data)
{
	bool result = false;
	if (mRTCAddr == 0) {
		LOGE("Address ds1307 is invalid!!!");
	}
	else {
		Wire.beginTransmission(mRTCAddr);
		Wire.write(reg);
		Wire.write(data);
		if (Wire.endTransmission() == 0) {
			LOGD("Set data successfully!");
			result = true;
		}
		else {
			LOGW("Set data failed!");
			result = false;
		};
	}
	delay(10);
	return result;
}

bool RTC::setTimeData(struct TimeDS1307 data)
{
	struct {
        uint8_t value;
        uint8_t reg;
        uint8_t min;
        uint8_t max;
        const char* name;
    } fields[] = {
        {data.second, REG_SEC, 0, 59, "SECOND"},
        {data.minute, REG_MIN, 0, 59, "MINUTE"},
        {data.hour, REG_HOUR, 0, 23, "HOUR"},
        {data.day, REG_DAY, 1, 7, "DAY"},
        {data.date, REG_DATE, 1, 31, "DATE"},
        {data.month, REG_MTH, 1, 12, "MONTH"},
        {static_cast<uint8_t>(data.year - 2000), REG_YEAR, 0, 99, "YEAR"} // Save as year - 2000
    };
	for (const auto& field : fields) {
        if (field.value < field.min || field.value > field.max) {
            LOGW("%s data is invalid!!!", field.name);
            return false;
        }

        LOGI("%s: %d", field.name, static_cast<int>(field.value));
        uint8_t valueToWrite = decToHex(field.value);
        if (!writeData(field.reg, valueToWrite)) {
            return false;
        }
    }
	return true;
}

bool RTC::setTimeLight(String light, struct TimeOfLight time, struct REG_TIME_LIGHT reg)
{
	struct {
		String typeLight;
		uint8_t reg;
		uint8_t data;
	} fiedls[] = {
		{"Switch", 	reg.SWITCH, time.sw},
		{"Hour", 	reg.HOUR, 	time.hour},
		{"Minute", 	reg.MINUTE, time.minute},
		{"Second", 	reg.SECOND, time.second},
	};
	bool result = false;
	for(int i = 0; i < 4; i++)
	{
		if(light == LISTLIGHT[i]) {
			LOGI("%s", light);
			result = true;
			break;
		}
	}
	if (!result) {
		return false;
		LOGE("Light does not support!!!");
	}
	else {
		for(const auto field : fiedls) {
			LOGI("%s: %d", field.typeLight, static_cast<int>(field.data));
			uint8_t valueToWrite = decToHex(field.data);
			if(!writeData(field.reg, valueToWrite)) {
				return false;
			}
		}
	}
	return true;
}

struct TimeOfLight RTC::getTimeLight(String light, uint8_t reg)
{
	struct TimeOfLight time { 0U, 0U, 0U, 0U};
	if (light == "Light1" || light == "Light2" || light == "Light3" || light == "Light4") {
		LOGI("%s reg %d", light, static_cast<int>(reg));
		getResponse(&time, reg);
	}
	else {
		LOGE("Light does not support!!!");
	}
	return time;
}

void RTC::getResponse(struct TimeOfLight *time, uint8_t REG)
{
	Wire.beginTransmission(mRTCAddr);
	Wire.write(REG);
	Wire.endTransmission();
	if (Wire.requestFrom(mRTCAddr, 4U) == 4U) {
		time->sw = bcdToDec(Wire.read());
		delay(10);
		time->hour = bcdToDec(Wire.read());
		delay(10);
		time->minute = bcdToDec(Wire.read());
		delay(10);
		time->second = bcdToDec(Wire.read());
		LOGI("%d %d:%d:%d", time->sw, time->hour, time->minute, time->second);
	}
	else {
		LOGE("Failed to get RTC time");
	}
}

void RTC::sendAllTimeData(const SignalType signal)
{
	const int size = 7U;
	int arr[size];
	arr[0] = static_cast<int>(mAllTimeData.second);
	arr[1] = static_cast<int>(mAllTimeData.minute);
	arr[2] = static_cast<int>(mAllTimeData.hour);
	arr[3] = static_cast<int>(mAllTimeData.day);
	arr[4] = static_cast<int>(mAllTimeData.date);
	arr[5] = static_cast<int>(mAllTimeData.month);
	arr[6] = static_cast<int>(mAllTimeData.year);
	Package package(arr, size);
	// LOGI("Send signal %d", signal);
	mRML->handleSignal(signal, &package);
}

void RTC::increaseValueOfTimeData()
{
	switch (mIndexOfAllTimeData)
	{
	case 0:
		if (mAllTimeData.second < 59U) {
			mAllTimeData.second++;
		}
		else if (mAllTimeData.second >= 59U) {
			mAllTimeData.second = 0U;
		}
		break;
	case 1:
		if (mAllTimeData.minute < 59U) {
			mAllTimeData.minute++;
		}
		else if (mAllTimeData.minute >= 59U) {
			mAllTimeData.minute = 0U;
		}
		break;
	case 2:
		if (mAllTimeData.hour < 23U) {
			mAllTimeData.hour++;
		}
		else if (mAllTimeData.hour >= 23U) {
			mAllTimeData.hour = 0U;
		}
		break;
	case 3:
		if (mAllTimeData.year < 2099U) {
			mAllTimeData.year++;
		}
		else if (mAllTimeData.year >= 2099U) {
			mAllTimeData.year = 2000U;
		}
		break;
	case 4:
		if (mAllTimeData.month < 12U) {
			mAllTimeData.month++;
		}
		else if (mAllTimeData.month >= 12U) {
			mAllTimeData.month = 1U;
		}
		break;
	case 5:
		if (mAllTimeData.date < 31U) {
			mAllTimeData.date++;
		}
		else if (mAllTimeData.date >= 31U) {
			mAllTimeData.date = 1U;
		}
		break;
	case 6:
		if (mAllTimeData.day < 7U) {
			mAllTimeData.day++;
		}
		else if (mAllTimeData.day >= 7U) {
			mAllTimeData.day = 1U;
		}
		break;
	default:
		break;
	}
}

void RTC::decreaseValueOfTimeData()
{
	switch (mIndexOfAllTimeData)
	{
	case 0:
		if (mAllTimeData.second > 0U) {
			mAllTimeData.second--;
		}
		else if (mAllTimeData.second == 0U) {
			mAllTimeData.second = 59;
		}
		break;
	case 1:
		if (mAllTimeData.minute > 0U) {
			mAllTimeData.minute--;
		}
		else if (mAllTimeData.minute == 0U) {
			mAllTimeData.minute = 59U;
		}
		break;
	case 2:
		if (mAllTimeData.hour > 0U) {
			mAllTimeData.hour--;
		}
		else if (mAllTimeData.hour == 0U) {
			mAllTimeData.hour = 23U;
		}
		break;
	case 3:
		if (mAllTimeData.year > 2000U) {
			mAllTimeData.year--;
		}
		else if (mAllTimeData.year <= 2000U) {
			mAllTimeData.year = 2099U;
		}
		break;
	case 4:
		if (mAllTimeData.month > 1U) {
			mAllTimeData.month--;
		}
		else if (mAllTimeData.month == 1U) {
			mAllTimeData.month = 12U;
		}
		break;
	case 5:
		if (mAllTimeData.date > 1U) {
			mAllTimeData.date--;
		}
		else if (mAllTimeData.date == 1U) {
			mAllTimeData.date = 31U;
		}
		break;
	case 6:
		if (mAllTimeData.day > 1U) {
			mAllTimeData.day--;
		}
		else if (mAllTimeData.day == 1U) {
			mAllTimeData.day = 7U;
		}
		break;
	default:
		break;
	}
}

void RTC::shiftIndexOfAllTimeData(bool adjust)
{
	if (adjust) {
		if (mIndexOfAllTimeData < 6U) {
			mIndexOfAllTimeData++;
		}
		else if (mIndexOfAllTimeData >= 6U) {
			mIndexOfAllTimeData = 0U;
		}
	}
	else {
		if (mIndexOfAllTimeData > 0U) {
			mIndexOfAllTimeData--;
		}
		else if (mIndexOfAllTimeData == 0U) {
			mIndexOfAllTimeData = 6U;
		}
	}
	LOGI("mIndexOfAllTimeData %d:", mIndexOfAllTimeData);
}

void RTC::increaseValueOfMenuMode()
{
	switch (mIndexLight)
	{
	case 0:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 0U;
		}
		break;
	case 1:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour < 23U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour >= 23U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour = 0U;
		}
		break;
	case 2:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute < 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute >= 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute = 0U;
		}
		break;
	case 3:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second < 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second >= 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second = 0U;
		}
		break;
	case 4:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 0U;
		}
		break;
	case 5:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour < 23U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour >= 23U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour = 0U;
		}
		break;
	case 6:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute < 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute >= 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute = 0U;
		}
		break;
	case 7:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second < 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second >= 59U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second = 0U;
		}
		break;
	default:
		break;
	}
}

void RTC::decreaseValueOfMenuMode()
{
	switch (mIndexLight)
	{
	case 0:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 0U;
		}
		break;
	case 1:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour > 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour = 23U;
		}
		break;
	case 2:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute > 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute = 59U;
		}
		break;
	case 3:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second > 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second = 59U;
		}
		break;
	case 4:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 0U;
		}
		break;
	case 5:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour > 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour = 23U;
		}
		break;
	case 6:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute > 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute = 59U;
		}
		break;
	case 7:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second > 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second == 0U) {
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second = 59U;
		}
		break;
	default:
		break;
	}
}

void RTC::sendTimeOfLight()
{
	int arr[9];
	arr[0] = mIndexListLight;
	arr[1] = mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw;
	arr[2] = mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour;
	arr[3] = mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute;
	arr[4] = mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second;
	arr[5] = mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw;
	arr[6] = mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour;
	arr[7] = mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute;
	arr[8] = mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second;
	Package data(arr, sizeof(arr) / sizeof(int));
	mRML->handleSignal(SignalType::LCD_MENU_MODE_OK, &data);
}

void RTC::sendAllTimeDataToWeb()
{
	const int size = sizeof(TimeDS1307) - 1;
	int data[size];
	data[0] = static_cast<int>(mAllTimeData.hour);
	data[1] = static_cast<int>(mAllTimeData.minute);
	data[2] = static_cast<int>(mAllTimeData.second);
	data[3] = static_cast<int>(mAllTimeData.day);
	data[4] = static_cast<int>(mAllTimeData.date);
	data[5] = static_cast<int>(mAllTimeData.month);
	data[6] = static_cast<int>(mAllTimeData.year);

	Package package(data, size);

	mRML->handleSignal(SignalType::WEB_GET_ALLTIME_DATA_RESPONSE, &package);
}

void RTC::checkConfigureTimeForLight()
{
	mAllTimeData = getTimeData();

	if(mCounterInstallIRButton > 0 && mCounterInstallIRButton <= 15) {
		mCounterInstallIRButton++;
	}
	if(mCounterInstallIRButton == 16) {
		mRML->handleSignal(SignalType::RTC_COUNTER_INSTALL_IRBUTTON_REACHED);
		mCounterInstallIRButton = 0;
	}

	std::map<String, LightMapValue>::iterator it;
	for (it = mTimeOfLight.begin(); it != mTimeOfLight.end(); it++)
	{
		if(it->second.first.second.sw != 0 && it->second.first.second.hour == mAllTimeData.hour &&
			it->second.first.second.minute == mAllTimeData.minute && it->second.first.second.second == mAllTimeData.second)
		{
			if(it->first == "Light1") {
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT1);
			}
			else if(it->first == "Light2") {
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT2);
			}
			else if(it->first == "Light3") {
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT3);
			}
			else if(it->first == "Light4") {
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT4);
			}
			else {
				/*Do nothing*/
			}
		}
		if(it->second.second.second.sw != 0 && it->second.second.second.hour == mAllTimeData.hour &&
			it->second.second.second.minute == mAllTimeData.minute && it->second.second.second.second == mAllTimeData.second)
		{
			if(it->first == "Light1") {
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT1);
			}
			else if(it->first == "Light2") {
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT2);
			}
			else if(it->first == "Light3") {
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT3);
			}
			else if(it->first == "Light4") {
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT4);
			}
			else {
				/*Do nothing*/
			}
		}
	}
}

void RTC::requestSetTimeAllData(Package *data)
{
	if(data->getSize() == 8)
    {
		int *parseData  = data->getPackage();
		TimeDS1307 data;
		data.hour        = parseData[1];
		data.minute      = parseData[2];
		data.second      = parseData[3];
        data.day         = parseData[4];
        data.date        = parseData[5];
        data.month       = parseData[6];
        data.year        = parseData[7];

		int result = false;
		result = setTimeData(data);
		if(result) {
			mRML->handleSignal(SignalType::WEB_SET_ALLTIME_DATA_RESPONSE);
		}
		else {
			LOGE("setTimeData ALLTIME DATA is incomplete!!!");
		}
	}
	else {
		LOGE("Lenght is invalid!!!");
	}
}

void RTC::sendLightDataToWeb(const SignalType signal)
{
	const int size = 8U;
	int order = mLightGetRequestResponse[signal].first;
	int data[size];
	data[0] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].first.second.sw);
	data[1] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].first.second.hour);
	data[2] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].first.second.minute);
	data[3] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].first.second.second);
	data[4] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].second.second.sw);
	data[5] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].second.second.hour);
	data[6] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].second.second.minute);
	data[7] = static_cast<int>(mTimeOfLight[LISTLIGHT[order]].second.second.second);

	Package package(data, size);
	mRML->handleSignal(mLightGetRequestResponse[signal].second, &package);
}

void RTC::requestSetLightData(Package *data, const SignalType signal)
{
	if(data->getSize() == 9U) {
		int *parseData  = data->getPackage();
		int order = mLightSetRequestResponse[signal].first;
		mTimeOfLight[LISTLIGHT[order]].first.second.sw 		= parseData[1];
        mTimeOfLight[LISTLIGHT[order]].first.second.hour 	= parseData[2];
        mTimeOfLight[LISTLIGHT[order]].first.second.minute 	= parseData[3];
        mTimeOfLight[LISTLIGHT[order]].first.second.second 	= parseData[4];
		mTimeOfLight[LISTLIGHT[order]].second.second.sw 	= parseData[5];
        mTimeOfLight[LISTLIGHT[order]].second.second.hour 	= parseData[6];
        mTimeOfLight[LISTLIGHT[order]].second.second.minute = parseData[7];
        mTimeOfLight[LISTLIGHT[order]].second.second.second = parseData[8];

		int result = false;
		result = setTimeLight(LISTLIGHT[order], mTimeOfLight[LISTLIGHT[order]].first.second, mTimeOfLight[LISTLIGHT[order]].first.first)
			   & setTimeLight(LISTLIGHT[order], mTimeOfLight[LISTLIGHT[order]].second.second, mTimeOfLight[LISTLIGHT[order]].second.first);
		if(result) {
			mRML->handleSignal(mLightSetRequestResponse[signal].second);
		}
		else {
			LOGE("setTimeLight LIGHT %d is incomplete!!!", order);
		}
	}
	else {
		LOGE("Lenght is invalid!!!");
	}
}

void RTC::receiveTimeDateFromNTP(Package *data)
{
	if(data->getSize() == 8) {
		int *parseData = data->getPackage();
		int checkSum = 0U;
		for(int i = 1; i < 8; i++) {
			checkSum += parseData[i];
		}
		if(checkSum <= 0U) {
			LOGE("Data from NTP is invalid! Double check connecting!");
		}
		else {
			// Format:        hour minute second day date month year
			mAllTimeData.hour 	= static_cast<uint8_t>(parseData[1]);
			mAllTimeData.minute = static_cast<uint8_t>(parseData[2]);
			mAllTimeData.second = static_cast<uint8_t>(parseData[3]);
			mAllTimeData.day 	= static_cast<uint8_t>(parseData[4]);
			mAllTimeData.date 	= static_cast<uint8_t>(parseData[5]);
			mAllTimeData.month 	= static_cast<uint8_t>(parseData[6]);
			mAllTimeData.year 	= static_cast<uint16_t>(parseData[7]);
			setTimeData(mAllTimeData);
		}
	}
	else {
		LOGE("Lenght is invalid!!!");
	}
}

void RTC::adjustTime() {
	mAllTimeData.second = mAllTimeData.second + 7;
	setTimeData(mAllTimeData);
}

void RTC::updateTimeForRTC() {
	if(mPreDate != mAllTimeData.date)
	{
		mPreDate = mAllTimeData.date;
		if(mFlagUpdateTIme == 0U) {
			mRML->handleSignal(SignalType::NETWORK_GET_TIME_DATE_FROM_NTP);
		}
		else if(mFlagUpdateTIme == mCounterUpdateTime) {
			adjustTime();
			mCounterUpdateTime = 0;
		}
		else {
			mCounterUpdateTime++;
		}
	}
}