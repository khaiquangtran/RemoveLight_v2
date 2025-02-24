#include "RTC.h"

RTC::RTC(RemoteLight *rml) : mRML(rml)
{
	LOGI("Initialization RTC!");

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

	mDS1307Addr = 0U;
	mAllTimeData = {0U, 0U, 0U, 0U, 0U, 0U, 0U};
	mIndexOfAllTimeData = 0U;
	mIndexListLight = 0U;
	mIndexLight = 0U;
	mCountRetry = 0U;

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
}

RTC::~RTC()
{
}

void RTC::handleSignal(const SignaLType signal, Package *data)
{
	if (mCountRetry >= RETRY)
	{
		LOGE("Can't connect to RTC");
		return;
	}
	else
	{
		// LOGD("Handle signal value: %d", signal);
		switch (signal)
		{
		case SignaLType::RTC_DISPLAY_ALL_TIME:
		{
			mAllTimeData = getTimeData();
			sendAllTimeData(SignaLType::LCD_DISPLAY_ALL_TIME);
			break;
		}
		case SignaLType::RTC_INCREASE_VALUE:
		{
			increaseValueOfTimeData();
			sendAllTimeData(SignaLType::LCD_DISPLAY_ALL_TIME);
			break;
		}
		case SignaLType::RTC_DECREASE_VALUE:
		{
			decreaseValueOfTimeData();
			sendAllTimeData(SignaLType::LCD_DISPLAY_ALL_TIME);
			break;
		}
		case SignaLType::RTC_SHIFT_LEFT_VALUE:
		{
			shiftIndexOfAllTimeData(true);
			break;
		}
		case SignaLType::RTC_SHIFT_RIGHT_VALUE:
		{
			shiftIndexOfAllTimeData(false);
			break;
		}
		case SignaLType::RTC_SETUP_MODE_OK:
		{
			setTimeData(mAllTimeData);
			mRML->handleSignal(SignaLType::REMOTE_LIGHT_END_SETUP_MODE);
			break;
		}
		case SignaLType::RTC_MOVE_LEFT_MENU_MODE:
		{
			if (mIndexListLight > 0U)
			{
				mIndexListLight--;
			}
			else if (mIndexListLight == 0U)
			{
				mIndexListLight = 3U;
			}
			int value = static_cast<int>(mIndexListLight);
			Package data(&value, 1);
			mRML->handleSignal(SignaLType::LCD_MOVE_LEFT_MENU_MODE, &data);
			break;
		}
		case SignaLType::RTC_MOVE_RIGHT_MENU_MODE:
		{
			if (mIndexListLight < 3U)
			{
				mIndexListLight++;
			}
			else if (mIndexListLight >= 3U)
			{
				mIndexListLight = 0U;
			}
			int value = static_cast<int>(mIndexListLight);
			Package data(&value, 1);
			mRML->handleSignal(SignaLType::LCD_MOVE_RIGHT_MENU_MODE, &data);
			break;
		}
		case SignaLType::RTC_MENU_MODE_OK:
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second = getTimeOfLight(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.first.SWITCH);
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second = getTimeOfLight(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.first.SWITCH);
			mIndexLight = 0U;
			sendTimeOfLight();
			break;
		}
		case SignaLType::RTC_INCREASE_VALUE_MENU_MODE:
		{
			increaseValueOfMenuMode();
			sendTimeOfLight();
			break;
		}
		case SignaLType::RTC_DECREASE_VALUE_MENU_MODE:
		{
			decreaseValueOfMenuMode();
			sendTimeOfLight();
			break;
		}
		case SignaLType::RTC_MOVE_RIGHT_INTO_MENU_MODE:
		{
			if (mIndexLight < 7U)
			{
				mIndexLight++;
			}
			else if (mIndexLight >= 7U)
			{
				mIndexLight = 0U;
			}
			break;
		}
		case SignaLType::RTC_MOVE_LEFT_INTO_MENU_MODE:
		{
			if (mIndexLight > 0U)
			{
				mIndexLight--;
			}
			else if (mIndexLight == 0U)
			{
				mIndexLight = 7U;
			}
			break;
		}
		case SignaLType::RTC_BACK_MENU_MODE:
		{
			// store data
			setTimeLight(LISTLIGHT[mIndexListLight], mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second, mTimeOfLight[LISTLIGHT[mIndexListLight]].first.first);
			setTimeLight(LISTLIGHT[mIndexListLight], mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second, mTimeOfLight[LISTLIGHT[mIndexListLight]].second.first);
			int value = static_cast<int>(mIndexListLight);
			Package data(&value, 1);
			mRML->handleSignal(SignaLType::LCD_MENU_MODE_BACK, &data);
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

bool RTC::checkAddress()
{
	if (scanAddress(DS1307_ADDR) == Hardware::INVALID)
	{
		LOGI("No find DS1307");
		return false;
	}
	else
	{
		mDS1307Addr = DS1307_ADDR;
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
	struct TimeDS1307 data
	{
		0U, 0U, 0U, 0U, 0U, 0U, 0U
	};
	if (mDS1307Addr == 0)
	{
		return data;
	}
	else
	{
		Wire.beginTransmission(mDS1307Addr);
		Wire.write(0x00);
		Wire.endTransmission();
		if (Wire.requestFrom(mDS1307Addr, 7U) == 7U)
		{
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
		else
		{
			LOGE("Failed to get RTC date and time");
			return data;
		};
	}
}

struct TimeOfLight RTC::getTimeOfLight(uint8_t reg)
{
	struct TimeOfLight data
	{
		0U, 0U, 0U, 0U
	};
	if (mDS1307Addr == 0)
	{
		return data;
	}
	else
	{
		Wire.beginTransmission(mDS1307Addr);
		Wire.write(reg);
		Wire.endTransmission();
		if (Wire.requestFrom(mDS1307Addr, 4U) == 4U)
		{
			data.sw = bcdToDec(Wire.read() & 0x7f);
			data.hour = bcdToDec(Wire.read());
			data.minute = bcdToDec(Wire.read() & 0x3f);
			data.second = bcdToDec(Wire.read());
			LOGI("%d %d:%d:%d", data.sw, data.hour, data.minute, data.second);
			return data;
		}
		else
		{
			LOGE("Failed to get RTC date and time");
			return data;
		};
	}
}

void RTC::writeData(uint8_t reg, uint8_t data)
{
	if (mDS1307Addr == 0)
	{
		LOGE("Address ds1307 is invalid!!!");
	}
	else
	{
		Wire.beginTransmission(mDS1307Addr);
		Wire.write(reg);
		Wire.write(data);
		if (Wire.endTransmission() == 0)
		{
			LOGD("Set data successfully!");
		}
		else
		{
			LOGW("Set data failed!");
		};
	}
	delay(10);
}

void RTC::setTimeData(struct TimeDS1307 data)
{
	if (data.second >= 0U && data.second <= 59U)
	{
		LOGI("SECOND");
		writeData(REG_SEC, decToHex(data.second));
	}
	else
	{
		LOGW("Second data is invalid!!!");
	}

	if (data.minute >= 0U && data.minute <= 59U)
	{
		LOGI("MINUTE");
		writeData(REG_MIN, decToHex(data.minute));
	}
	else
	{
		LOGW("Minute data is invalid!!!");
	}

	if (data.hour >= 1U && data.hour <= 23U)
	{
		LOGI("HOUR");
		writeData(REG_HOUR, decToHex(data.hour));
	}
	else
	{
		LOGW("Hour data is invalid!!!");
	}

	if (data.day >= 1U && data.day <= 7U)
	{
		LOGI("DAY");
		writeData(REG_DAY, data.day);
	}
	else
	{
		LOGW("Day data is invalid!!!");
	}

	if (data.date >= 1U && data.date <= 31U)
	{
		LOGI("DATE");
		writeData(REG_DATE, decToHex(data.date));
	}
	else
	{
		LOGW("Date data is invalid!!!");
	}

	if (data.month >= 1U && data.month <= 12U)
	{
		LOGI("MONTH");
		writeData(REG_MTH, decToHex(data.month));
	}
	else
	{
		LOGW("Month data is invalid!!!");
	}

	if (data.year >= 2000U && data.year <= 2099U)
	{
		LOGI("YEAR");
		writeData(REG_YEAR, decToHex(data.year - 2000U));
	}
	else
	{
		LOGW("Year data is invalid!!!");
	}
}

void RTC::setTimeLight(String light, struct TimeOfLight time, struct REG_TIME_LIGHT reg)
{
	if (light == "Light1" || light == "Light2" || light == "Light3" || light == "Light4")
	{
		LOGI("%s", light);
		LOGI("Switch");
		writeData(reg.SWITCH, decToHex(time.sw));
		LOGI("hour");
		writeData(reg.HOUR, decToHex(time.hour));
		LOGI("minute");
		writeData(reg.MINUTE, decToHex(time.minute));
		LOGI("second");
		writeData(reg.SECOND, decToHex(time.second));
	}
	else
	{
		LOGE("Light does not support!!!");
	}
}

struct TimeOfLight RTC::getTimeLight(String light, uint8_t reg)
{
	struct TimeOfLight time
	{
		0U, 0U, 0U, 0U
	};
	if (light == "Light1" || light == "Light2" || light == "Light3" || light == "Light4")
	{
		LOGI("%s reg %d", light, reg);
		getResponse(&time, reg);
	}
	else
	{
		LOGE("Light does not support!!!");
	}
	return time;
}

void RTC::getResponse(struct TimeOfLight *time, uint8_t REG)
{
	Wire.beginTransmission(mDS1307Addr);
	Wire.write(REG);
	Wire.endTransmission();
	if (Wire.requestFrom(mDS1307Addr, 4U) == 4U)
	{
		time->sw = bcdToDec(Wire.read());
		delay(10);
		time->hour = bcdToDec(Wire.read());
		delay(10);
		time->minute = bcdToDec(Wire.read());
		delay(10);
		time->second = bcdToDec(Wire.read());
		LOGI("%d %d:%d:%d", time->sw, time->hour, time->minute, time->second);
	}
	else
	{
		LOGE("Failed to get RTC time");
	}
}

void RTC::sendAllTimeData(const SignaLType signal)
{
	int arr[7];
	arr[0] = static_cast<int>(mAllTimeData.second);
	arr[1] = static_cast<int>(mAllTimeData.minute);
	arr[2] = static_cast<int>(mAllTimeData.hour);
	arr[3] = static_cast<int>(mAllTimeData.day);
	arr[4] = static_cast<int>(mAllTimeData.date);
	arr[5] = static_cast<int>(mAllTimeData.month);
	arr[6] = static_cast<int>(mAllTimeData.year);
	Package package(arr, sizeof(arr) / sizeof(int));
	// LOGI("Send signal %d", signal);
	mRML->handleSignal(signal, &package);
}

void RTC::increaseValueOfTimeData()
{
	switch (mIndexOfAllTimeData)
	{
	case 0:
	{
		if (mAllTimeData.second < 59U)
		{
			mAllTimeData.second++;
		}
		else if (mAllTimeData.second >= 59U)
		{
			mAllTimeData.second = 0U;
		}
		break;
	}
	case 1:
	{
		if (mAllTimeData.minute < 59U)
		{
			mAllTimeData.minute++;
		}
		else if (mAllTimeData.minute >= 59U)
		{
			mAllTimeData.minute = 0U;
		}
		break;
	}
	case 2:
	{
		if (mAllTimeData.hour < 23U)
		{
			mAllTimeData.hour++;
		}
		else if (mAllTimeData.hour >= 23U)
		{
			mAllTimeData.hour = 0U;
		}
		break;
	}
	case 3:
	{
		if (mAllTimeData.year < 2099U)
		{
			mAllTimeData.year++;
		}
		else if (mAllTimeData.year >= 2099U)
		{
			mAllTimeData.year = 2000U;
		}
		break;
	}
	case 4:
	{
		if (mAllTimeData.month < 12U)
		{
			mAllTimeData.month++;
		}
		else if (mAllTimeData.month >= 12U)
		{
			mAllTimeData.month = 1U;
		}
		break;
	}
	case 5:
	{
		if (mAllTimeData.date < 31U)
		{
			mAllTimeData.date++;
		}
		else if (mAllTimeData.date >= 31U)
		{
			mAllTimeData.date = 1U;
		}
		break;
	}
	case 6:
	{
		if (mAllTimeData.day < 7U)
		{
			mAllTimeData.day++;
		}
		else if (mAllTimeData.day >= 7U)
		{
			mAllTimeData.day = 1U;
		}
		break;
	}
	default:
		break;
	}
}

void RTC::decreaseValueOfTimeData()
{
	switch (mIndexOfAllTimeData)
	{
	case 0:
	{
		if (mAllTimeData.second > 0U)
		{
			mAllTimeData.second--;
		}
		else if (mAllTimeData.second == 0U)
		{
			mAllTimeData.second = 59;
		}
		break;
	}
	case 1:
	{
		if (mAllTimeData.minute > 0U)
		{
			mAllTimeData.minute--;
		}
		else if (mAllTimeData.minute == 0U)
		{
			mAllTimeData.minute = 59U;
		}
		break;
	}
	case 2:
	{
		if (mAllTimeData.hour > 0U)
		{
			mAllTimeData.hour--;
		}
		else if (mAllTimeData.hour == 0U)
		{
			mAllTimeData.hour = 23U;
		}
		break;
	}
	case 3:
	{
		if (mAllTimeData.year > 2000U)
		{
			mAllTimeData.year--;
		}
		else if (mAllTimeData.year <= 2000U)
		{
			mAllTimeData.year = 2099U;
		}

		break;
	}
	case 4:
	{
		if (mAllTimeData.month > 1U)
		{
			mAllTimeData.month--;
		}
		else if (mAllTimeData.month == 1U)
		{
			mAllTimeData.month = 12U;
		}
		break;
	}
	case 5:
	{
		if (mAllTimeData.date > 1U)
		{
			mAllTimeData.date--;
		}
		else if (mAllTimeData.date == 1U)
		{
			mAllTimeData.date = 31U;
		}
		break;
	}
	case 6:
	{
		if (mAllTimeData.day > 1U)
		{
			mAllTimeData.day--;
		}
		else if (mAllTimeData.day == 1U)
		{
			mAllTimeData.day = 7U;
		}
		break;
	}
	default:
		break;
	}
}

void RTC::shiftIndexOfAllTimeData(bool adjust)
{
	if (adjust)
	{
		if (mIndexOfAllTimeData < 6U)
		{
			mIndexOfAllTimeData++;
		}
		else if (mIndexOfAllTimeData >= 6U)
		{
			mIndexOfAllTimeData = 0U;
		}
	}
	else
	{
		if (mIndexOfAllTimeData > 0U)
		{
			mIndexOfAllTimeData--;
		}
		else if (mIndexOfAllTimeData == 0U)
		{
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
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 0U;
		}
		break;
	}
	case 1:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour < 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour >= 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour = 0U;
		}
		break;
	}
	case 2:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute = 0U;
		}
		break;
	}
	case 3:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second = 0U;
		}
		break;
	}
	case 4:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 0U;
		}
		break;
	}
	case 5:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour < 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour >= 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour = 0U;
		}
		break;
	}
	case 6:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute = 0U;
		}
		break;
	}
	case 7:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second = 0U;
		}
		break;
	}
	default:
		break;
	}
}

void RTC::decreaseValueOfMenuMode()
{
	switch (mIndexLight)
	{
	case 0:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.sw = 0U;
		}
		break;
	}
	case 1:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.hour = 23U;
		}
		break;
	}
	case 2:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.minute = 59U;
		}
		break;
	}
	case 3:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second.second = 59U;
		}
		break;
	}
	case 4:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.sw = 0U;
		}
		break;
	}
	case 5:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.hour = 23U;
		}
		break;
	}
	case 6:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.minute = 59U;
		}
		break;
	}
	case 7:
	{
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second.second = 59U;
		}
		break;
	}
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
	mRML->handleSignal(SignaLType::LCD_MENU_MODE_OK, &data);
}