#include "RTC.h"

RTC::RTC(std::shared_ptr<RemoteLight> rml) : mRML(rml), mRTCAddr(0U), mIndexOfAllTimeData(0U), mIndexLight(0U),
											 mIndexListLight(0U), mCountRetry(0U), mFlagUpdateTIme(0U), mPreDate(0U), mCounterUpdateTime(0U)
{
	mTimeOfLight["Light1"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});
	mTimeOfLight["Light2"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});
	mTimeOfLight["Light3"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});
	mTimeOfLight["Light4"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});

	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT1_DATA_REQUEST] = std::make_pair(0, SignalType::WEB_GET_LIGHT1_DATA_RESPONSE);
	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT2_DATA_REQUEST] = std::make_pair(1, SignalType::WEB_GET_LIGHT2_DATA_RESPONSE);
	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT3_DATA_REQUEST] = std::make_pair(2, SignalType::WEB_GET_LIGHT3_DATA_RESPONSE);
	mLightGetRequestResponse[SignalType::WEB_GET_LIGHT4_DATA_REQUEST] = std::make_pair(3, SignalType::WEB_GET_LIGHT4_DATA_RESPONSE);

	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT1_DATA_REQUEST] = std::make_pair(0, SignalType::WEB_SET_LIGHT1_DATA_RESPONSE);
	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT2_DATA_REQUEST] = std::make_pair(1, SignalType::WEB_SET_LIGHT2_DATA_RESPONSE);
	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT3_DATA_REQUEST] = std::make_pair(2, SignalType::WEB_SET_LIGHT3_DATA_RESPONSE);
	mLightSetRequestResponse[SignalType::WEB_SET_LIGHT4_DATA_REQUEST] = std::make_pair(3, SignalType::WEB_SET_LIGHT4_DATA_RESPONSE);

	mAllTimeData = {0U, 0U, 0U, 0U, 0U, 0U, 0U};
	mTimeTemp = {0U, 0U, 0U, 0U};

	LOGI(" =========== RTC ===========");
}

void RTC::init()
{
	#if NOT_CONNECT_DEVICE
		LOGW("Skip init RTC due to NOT_CONNECT_DEVICE is defined");
		return;
	#endif

	#ifndef INIT_I2C
	#define INIT_I2C
		Wire.begin();
	#endif
	retry:
		if (checkAddress())
		{
			// std::map<String, LightMapValue>::iterator it;
			// for (it = mTimeOfLight.begin(); it != mTimeOfLight.end(); it++)
			// {
			// 	it->second.first.second = getTimeLight(it->first, it->second.first.first.SWITCH);
			// 	it->second.second.second = getTimeLight(it->first, it->second.second.first.SWITCH);
			// }
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

void RTC::handleSignal(const SignalType &signal, const Package *data)
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
		case SignalType::RTC_DISPLAY_ALL_TIME:
		{
			mAllTimeData = getTimeData();
			sendAllTimeData(SignalType::LCD_DISPLAY_ALL_TIME);
			break;
		}
		case SignalType::RTC_INCREASE_VALUE:
		{
			increaseValueOfTimeData();
			sendAllTimeData(SignalType::LCD_DISPLAY_ALL_TIME);
			break;
		}
		case SignalType::RTC_DECREASE_VALUE:
		{
			decreaseValueOfTimeData();
			sendAllTimeData(SignalType::LCD_DISPLAY_ALL_TIME);
			break;
		}
		case SignalType::RTC_SHIFT_LEFT_VALUE:
		{
			shiftIndexOfAllTimeData(true);
			break;
		}
		case SignalType::RTC_SHIFT_RIGHT_VALUE:
		{
			shiftIndexOfAllTimeData(false);
			break;
		}
		case SignalType::RTC_SETUP_MODE_OK:
		{
			setTimeData(mAllTimeData);
			break;
		}
		case SignalType::RTC_MOVE_LEFT_MENU_MODE:
		{
			if (mIndexListLight > 0U)
			{
				mIndexListLight--;
			}
			else if (mIndexListLight == 0U)
			{
				mIndexListLight = 3U;
			}
			std::vector<int32_t> vecData = {static_cast<int32_t>(mIndexListLight)};
			std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
			// mRML->handleSignal(SignalType::LCD_MOVE_LEFT_MENU_MODE, &data);
			break;
		}
		case SignalType::RTC_MOVE_RIGHT_MENU_MODE:
		{
			if (mIndexListLight < 3U)
			{
				mIndexListLight++;
			}
			else if (mIndexListLight >= 3U)
			{
				mIndexListLight = 0U;
			}
			std::vector<int32_t> vecData = {static_cast<int32_t>(mIndexListLight)};
			std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
			// mRML->handleSignal(SignalType::LCD_MOVE_RIGHT_MENU_MODE, &data);
			break;
		}
		case SignalType::RTC_MENU_MODE_OK:
		{
			// mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second = getTimeOfLight(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.first.SWITCH);
			// mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second = getTimeOfLight(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.first.SWITCH);
			mIndexLight = 0U;
			sendTimeOfLight();
			break;
		}
		case SignalType::RTC_INCREASE_VALUE_MENU_MODE:
		{
			increaseValueOfMenuMode();
			sendTimeOfLight();
			break;
		}
		case SignalType::RTC_DECREASE_VALUE_MENU_MODE:
		{
			decreaseValueOfMenuMode();
			sendTimeOfLight();
			break;
		}
		case SignalType::RTC_MOVE_RIGHT_INTO_MENU_MODE:
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
		case SignalType::RTC_MOVE_LEFT_INTO_MENU_MODE:
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
		case SignalType::RTC_BACK_MENU_MODE:
		{
			// store data
			// setTimeLight(LISTLIGHT[mIndexListLight], mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second, mTimeOfLight[LISTLIGHT[mIndexListLight]].first.first);
			// setTimeLight(LISTLIGHT[mIndexListLight], mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second, mTimeOfLight[LISTLIGHT[mIndexListLight]].second.first);
			std::vector<int32_t> vecData = {static_cast<int32_t>(mIndexListLight)};
			std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
			mRML->handleSignal(SignalType::LCD_MENU_MODE_BACK, packData.get());
			break;
		}
		case SignalType::WEB_GET_ALLTIME_DATA_REQUEST:
		{
			sendAllTimeDataToWeb();
			break;
		}
		case SignalType::RTC_CHECK_CONFIGURED_TIME_FOR_LIGHT:
		{
			checkConfigureTimeForLight();
			updateTimeForRTC();
			break;
		}
		case SignalType::WEB_SET_ALLTIME_DATA_REQUEST:
		{
			requestSetTimeAllData(data);
			break;
		}
		case SignalType::WEB_GET_LIGHT1_DATA_REQUEST:
		case SignalType::WEB_GET_LIGHT2_DATA_REQUEST:
		case SignalType::WEB_GET_LIGHT3_DATA_REQUEST:
		case SignalType::WEB_GET_LIGHT4_DATA_REQUEST:
		{
			sendLightDataToWeb(signal);
			break;
		}
		case SignalType::WEB_SET_LIGHT1_DATA_REQUEST:
		case SignalType::WEB_SET_LIGHT2_DATA_REQUEST:
		case SignalType::WEB_SET_LIGHT3_DATA_REQUEST:
		case SignalType::WEB_SET_LIGHT4_DATA_REQUEST:
		{
			requestSetLightData(data, signal);
			break;
		}
		case SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP:
		{
			receiveTimeDateFromNTP(data);
			break;
		}
		case SignalType::RTC_GET_ALL_ALL:
		{
			mAllTimeData = getTimeData();
			mPreDate = mAllTimeData.date;
			break;
		}
		case SignalType::RTC_SET_FLAG_UPDATE_TIME_WITH_NTP_SUCCESS:
		{
			LOGI("RTC will update time from NTP server");
			mFlagUpdateTIme = 0U;
			break;
		}
		case SignalType::RTC_SET_FLAG_UPDATE_TIME_WITH_NTP_FAILED:
		{
			LOGW("RTC failed to update time from NTP server");
			mFlagUpdateTIme = 3U;
			mCounterUpdateTime = 0U;
			break;
		}
		case SignalType::EEPROM_SEND_DATA_TO_RTC:
		{
			parseTimeDataFromEEPROM(data);
			break;
		}
		case SignalType::RTC_GET_LIGHT_ON_OFF_DATA:
		{
			getLightOnOffDataFromEEPROM(data);
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
	if (scanAddress(RTC_ADDRESS) == Hardware::INVALID)
	{
		LOGI("No find RTC");
		return false;
	}
	else
	{
		mRTCAddr = RTC_ADDRESS;
		LOGI("Find out RTC");
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
	static uint8_t sec = 0;
	struct TimeDS1307 data{0U, 0U, 0U, 0U, 0U, 0U, 0U};
	#if NOT_CONNECT_DEVICE
		LOGW("getTimeData is dummy data due to NOT_CONNECT_DEVICE is defined");
		data.second = sec++;
		data.minute = 0U;
		data.hour = 12U;
		data.day = 3U;
		data.date = 25U;
		data.month = 8U;
		data.year = 2025U;
		return data;
	#endif
	if (mRTCAddr == 0)
	{
		return data;
	}
	else
	{
		Wire.beginTransmission(mRTCAddr);
		Wire.write(0x00);
		Wire.endTransmission();
		if (Wire.requestFrom(mRTCAddr, 7U) == 7U)
		{
			data.second = bcdToDec(Wire.read() & 0x7f);
			data.minute = bcdToDec(Wire.read());
			data.hour = bcdToDec(Wire.read() & 0x3f);
			data.day = bcdToDec(Wire.read());
			data.date = bcdToDec(Wire.read());
			data.month = bcdToDec(Wire.read());
			data.year = bcdToDec(Wire.read()) + 2000;
			// LOGI("%d/%d/%d %d %d:%d:%d", data.date, data.month, data.year, data.day, data.hour, data.minute, data.second);
			return data;
		}
		else
		{
			LOGE("Failed to get RTC date and time");
			return data;
		};
	}
}

bool RTC::writeData(uint8_t reg, uint8_t data)
{
	#if NOT_CONNECT_DEVICE
		LOGW("writeData is skipped due to NOT_CONNECT_DEVICE is defined");
		return true;
	#endif
	bool result = false;
	if (mRTCAddr == 0)
	{
		LOGE("Address ds1307 is invalid!!!");
	}
	else
	{
		Wire.beginTransmission(mRTCAddr);
		Wire.write(reg);
		Wire.write(data);
		if (Wire.endTransmission() == 0)
		{
			LOGD("Set data successfully!");
			result = true;
		}
		else
		{
			LOGW("Set data failed!");
			result = false;
		};
	}
	return result;
}

bool RTC::setTimeData(struct TimeDS1307 data)
{
	#if NOT_CONNECT_DEVICE
		LOGW("setTimeData is skipped due to NOT_CONNECT_DEVICE is defined");
		return true;
	#endif
	struct
	{
		uint8_t value;
		uint8_t reg;
		uint8_t min;
		uint8_t max;
		const char *name;
	} fields[] = {
		{data.second, REG_SEC, 0, 59, "SECOND"},
		{data.minute, REG_MIN, 0, 59, "MINUTE"},
		{data.hour, REG_HOUR, 0, 23, "HOUR"},
		{data.day, REG_DAY, 1, 7, "DAY"},
		{data.date, REG_DATE, 1, 31, "DATE"},
		{data.month, REG_MTH, 1, 12, "MONTH"},
		{static_cast<uint8_t>(data.year - 2000), REG_YEAR, 0, 99, "YEAR"} // Save as year - 2000
	};
	for (const auto &field : fields)
	{
		if (field.value < field.min || field.value > field.max)
		{
			LOGW("%s data is invalid!!!", field.name);
			return false;
		}
		LOGI("%s: %d", field.name, static_cast<int32_t>(field.value));
		uint8_t valueToWrite = decToHex(field.value);
		if (!writeData(field.reg, valueToWrite))
		{
			return false;
		}
	}
	return true;
}

void RTC::sendAllTimeData(const SignalType signal)
{
	std::vector<int32_t> vecData = {
        static_cast<int32_t>(mAllTimeData.second),
        static_cast<int32_t>(mAllTimeData.minute),
        static_cast<int32_t>(mAllTimeData.hour),
        static_cast<int32_t>(mAllTimeData.day),
        static_cast<int32_t>(mAllTimeData.date),
        static_cast<int32_t>(mAllTimeData.month),
        static_cast<int32_t>(mAllTimeData.year)
    };
	std::unique_ptr<Package> package = std::make_unique<Package>(vecData);
	LOGI("%d %d/%d/%d", vecData[3], vecData[4], vecData[5], vecData[6]);
	LOGI("%d:%d:%d", vecData[0], vecData[1], vecData[2]);
	mRML->handleSignal(signal, package.get());
}

void RTC::increaseValueOfTimeData()
{
	switch (mIndexOfAllTimeData)
	{
	case 0:
		if (mAllTimeData.second < 59U)
		{
			mAllTimeData.second++;
		}
		else if (mAllTimeData.second >= 59U)
		{
			mAllTimeData.second = 0U;
		}
		break;
	case 1:
		if (mAllTimeData.minute < 59U)
		{
			mAllTimeData.minute++;
		}
		else if (mAllTimeData.minute >= 59U)
		{
			mAllTimeData.minute = 0U;
		}
		break;
	case 2:
		if (mAllTimeData.hour < 23U)
		{
			mAllTimeData.hour++;
		}
		else if (mAllTimeData.hour >= 23U)
		{
			mAllTimeData.hour = 0U;
		}
		break;
	case 3:
		if (mAllTimeData.year < 2099U)
		{
			mAllTimeData.year++;
		}
		else if (mAllTimeData.year >= 2099U)
		{
			mAllTimeData.year = 2000U;
		}
		break;
	case 4:
		if (mAllTimeData.month < 12U)
		{
			mAllTimeData.month++;
		}
		else if (mAllTimeData.month >= 12U)
		{
			mAllTimeData.month = 1U;
		}
		break;
	case 5:
		if (mAllTimeData.date < 31U)
		{
			mAllTimeData.date++;
		}
		else if (mAllTimeData.date >= 31U)
		{
			mAllTimeData.date = 1U;
		}
		break;
	case 6:
		if (mAllTimeData.day < 7U)
		{
			mAllTimeData.day++;
		}
		else if (mAllTimeData.day >= 7U)
		{
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
		if (mAllTimeData.second > 0U)
		{
			mAllTimeData.second--;
		}
		else if (mAllTimeData.second == 0U)
		{
			mAllTimeData.second = 59;
		}
		break;
	case 1:
		if (mAllTimeData.minute > 0U)
		{
			mAllTimeData.minute--;
		}
		else if (mAllTimeData.minute == 0U)
		{
			mAllTimeData.minute = 59U;
		}
		break;
	case 2:
		if (mAllTimeData.hour > 0U)
		{
			mAllTimeData.hour--;
		}
		else if (mAllTimeData.hour == 0U)
		{
			mAllTimeData.hour = 23U;
		}
		break;
	case 3:
		if (mAllTimeData.year > 2000U)
		{
			mAllTimeData.year--;
		}
		else if (mAllTimeData.year <= 2000U)
		{
			mAllTimeData.year = 2099U;
		}
		break;
	case 4:
		if (mAllTimeData.month > 1U)
		{
			mAllTimeData.month--;
		}
		else if (mAllTimeData.month == 1U)
		{
			mAllTimeData.month = 12U;
		}
		break;
	case 5:
		if (mAllTimeData.date > 1U)
		{
			mAllTimeData.date--;
		}
		else if (mAllTimeData.date == 1U)
		{
			mAllTimeData.date = 31U;
		}
		break;
	case 6:
		if (mAllTimeData.day > 1U)
		{
			mAllTimeData.day--;
		}
		else if (mAllTimeData.day == 1U)
		{
			mAllTimeData.day = 7U;
		}
		break;
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
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 0U;
		}
		break;
	case 1:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour < 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour >= 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour = 0U;
		}
		break;
	case 2:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute = 0U;
		}
		break;
	case 3:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second = 0U;
		}
		break;
	case 4:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 0U;
		}
		break;
	case 5:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour < 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour >= 23U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour = 0U;
		}
		break;
	case 6:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute = 0U;
		}
		break;
	case 7:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second < 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second++;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second >= 59U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second = 0U;
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
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 0U;
		}
		break;
	case 1:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour = 23U;
		}
		break;
	case 2:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute = 59U;
		}
		break;
	case 3:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second = 59U;
		}
		break;
	case 4:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 1U;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 0U;
		}
		break;
	case 5:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour = 23U;
		}
		break;
	case 6:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute = 59U;
		}
		break;
	case 7:
		if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second > 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second--;
		}
		else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second == 0U)
		{
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second = 59U;
		}
		break;
	default:
		break;
	}
}

void RTC::sendTimeOfLight()
{
	const auto &time = mTimeOfLight[LISTLIGHT[mIndexListLight]];
    std::vector<int32_t> vecData = {
        static_cast<int32_t>(mIndexListLight),
        static_cast<int32_t>(time.first.sw),
        static_cast<int32_t>(time.first.hour),
        static_cast<int32_t>(time.first.minute),
        static_cast<int32_t>(time.first.second),
        static_cast<int32_t>(time.second.sw),
        static_cast<int32_t>(time.second.hour),
        static_cast<int32_t>(time.second.minute),
        static_cast<int32_t>(time.second.second)
    };
	std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
	// mRML->handleSignal(SignalType::LCD_MENU_MODE_OK, &data);
}

void RTC::sendAllTimeDataToWeb()
{
    std::vector<int32_t> vecData = {
        static_cast<int32_t>(mAllTimeData.hour),
        static_cast<int32_t>(mAllTimeData.minute),
        static_cast<int32_t>(mAllTimeData.second),
        static_cast<int32_t>(mAllTimeData.day),
        static_cast<int32_t>(mAllTimeData.date),
        static_cast<int32_t>(mAllTimeData.month),
        static_cast<int32_t>(mAllTimeData.year)
    };

    std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
    // mRML->handleSignal(SignalType::WEB_GET_ALLTIME_DATA_RESPONSE, packData.get());
}


void RTC::checkConfigureTimeForLight()
{
	mAllTimeData = getTimeData();

	std::map<String, LightMapValue>::iterator it;
	for (it = mTimeOfLight.begin(); it != mTimeOfLight.end(); it++)
	{
		if (it->second.first.sw != 0
			&& it->second.first.hour 	== mAllTimeData.hour
			&& it->second.first.minute 	== mAllTimeData.minute
			&& it->second.first.second 	== mAllTimeData.second)
		{
			if (it->first == "Light1")
			{
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT1);
			}
			else if (it->first == "Light2")
			{
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT2);
			}
			else if (it->first == "Light3")
			{
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT3);
			}
			else if (it->first == "Light4")
			{
				mRML->handleSignal(SignalType::RTC_TURN_ON_LIGHT4);
			}
			else
			{
				/*Do nothing*/
			}
		}
		if (it->second.second.sw != 0
			&& it->second.second.hour 	== mAllTimeData.hour
			&& it->second.second.minute == mAllTimeData.minute
			&& it->second.second.second == mAllTimeData.second)
		{
			if (it->first == "Light1")
			{
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT1);
			}
			else if (it->first == "Light2")
			{
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT2);
			}
			else if (it->first == "Light3")
			{
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT3);
			}
			else if (it->first == "Light4")
			{
				mRML->handleSignal(SignalType::RTC_TURN_OFF_LIGHT4);
			}
			else
			{
				/*Do nothing*/
			}
		}
	}
}

void RTC::requestSetTimeAllData(const Package *data)
{
	if (data->getSize() == 8)
	{
		const int32_t *parseData = data->getPackage();
		TimeDS1307 data;
		data.hour = parseData[1];
		data.minute = parseData[2];
		data.second = parseData[3];
		data.day = parseData[4];
		data.date = parseData[5];
		data.month = parseData[6];
		data.year = parseData[7];

		int32_t result = false;
		result = setTimeData(data);
		if (result)
		{
			// mRML->handleSignal(SignalType::WEB_SET_ALLTIME_DATA_RESPONSE);
		}
		else
		{
			LOGE("setTimeData ALLTIME DATA is incomplete!!!");
		}
	}
	else
	{
		LOGE("Lenght is invalid!!!");
	}
}

void RTC::sendLightDataToWeb(const SignalType signal)
{
    const int32_t order = mLightGetRequestResponse[signal].first;

    std::vector<int32_t> vecData = {
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].first.sw),
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].first.hour),
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].first.minute),
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].first.second),
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].second.sw),
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].second.hour),
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].second.minute),
        static_cast<int32_t>(mTimeOfLight[LISTLIGHT[order]].second.second)
    };

    std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
    // mRML->handleSignal(mLightGetRequestResponse[signal].second, packData.get());
}

void RTC::requestSetLightData(const Package *data, const SignalType &signal)
{
	if (data->getSize() == 9U)
	{
		const int32_t *parseData = data->getPackage();
		const int32_t order = mLightSetRequestResponse[signal].first;
		mTimeOfLight[LISTLIGHT[order]].first.sw = parseData[1];
		mTimeOfLight[LISTLIGHT[order]].first.hour = parseData[2];
		mTimeOfLight[LISTLIGHT[order]].first.minute = parseData[3];
		mTimeOfLight[LISTLIGHT[order]].first.second = parseData[4];
		mTimeOfLight[LISTLIGHT[order]].second.sw = parseData[5];
		mTimeOfLight[LISTLIGHT[order]].second.hour = parseData[6];
		mTimeOfLight[LISTLIGHT[order]].second.minute = parseData[7];
		mTimeOfLight[LISTLIGHT[order]].second.second = parseData[8];

		int32_t result = false;
		// result = setTimeLight(LISTLIGHT[order], mTimeOfLight[LISTLIGHT[order]].first.second, mTimeOfLight[LISTLIGHT[order]].first.first)
		// 	   & setTimeLight(LISTLIGHT[order], mTimeOfLight[LISTLIGHT[order]].second.second, mTimeOfLight[LISTLIGHT[order]].second.first);
		if (result)
		{
			// mRML->handleSignal(mLightSetRequestResponse[signal].second);
		}
		else
		{
			LOGE("setTimeLight LIGHT %d is incomplete!!!", order);
		}
	}
	else
	{
		LOGE("Lenght is invalid!!!");
	}
}

void RTC::receiveTimeDateFromNTP(const Package *data)
{
	if (data->getSize() == 7)
	{
		const int32_t *parseData = data->getPackage();
		int32_t checkSum = 0U;
		for (int32_t i = 1; i < 7; i++)
		{
			checkSum += parseData[i];
		}
		if (checkSum <= 0U)
		{
			LOGE("Data from NTP is invalid! Double check connecting!");
		}
		else
		{
			// Format:        hour minute second day date month year
			mAllTimeData.hour = static_cast<uint8_t>(parseData[0]);
			mAllTimeData.minute = static_cast<uint8_t>(parseData[1]);
			mAllTimeData.second = static_cast<uint8_t>(parseData[2]);
			mAllTimeData.day = static_cast<uint8_t>(parseData[3]);
			mAllTimeData.date = static_cast<uint8_t>(parseData[4]);
			mAllTimeData.month = static_cast<uint8_t>(parseData[5]);
			mAllTimeData.year = static_cast<uint16_t>(parseData[6]);
			setTimeData(mAllTimeData);
		}
	}
	else
	{
		LOGE("Lenght is invalid!!!");
	}
}

void RTC::adjustTime()
{
	mAllTimeData.second = mAllTimeData.second + 7;
	setTimeData(mAllTimeData);
}

void RTC::updateTimeForRTC()
{
	if (mPreDate != mAllTimeData.date)
	{
		LOGI(".");
		mPreDate = mAllTimeData.date;
		if (mFlagUpdateTIme == 0U)
		{
			// mRML->handleSignal(SignalType::NETWORK_GET_TIME_DATE_FROM_NTP);
		}
		else if (mFlagUpdateTIme == mCounterUpdateTime)
		{
			adjustTime();
			mCounterUpdateTime = 0;
			// mRML->handleSignal(SignalType::TASKS_CONNECT_RETRY);
		}
		else
		{
			mCounterUpdateTime++;
		}
	}
}

void RTC::parseTimeDataFromEEPROM(const Package *data)
{
	if (data == nullptr)
	{
		LOGE("Data from EEPROM is null.");
		return;
	}
	else
	{
		const int32_t size = data->getSize();
		const int32_t *value = data->getPackage();
		if (size != 4)
		{
			LOGE("Data from EEPROM with length is not 12.");
			return;
		}
		else
		{
			mTimeTemp = {static_cast<uint8_t>(value[0]), static_cast<uint8_t>(value[1]),
						 static_cast<uint8_t>(value[2]), static_cast<uint8_t>(value[3])};
		}
	}
}

void RTC::getLightOnOffDataFromEEPROM(const Package* data)
{
	if (data == nullptr)
	{
		LOGE("Data from EEPROM is null.");
		return;
	}
	else
	{
		const int32_t size = data->getSize();
		const int32_t *value = data->getPackage();
		if (size == 0)
		{
			LOGE("Data from EEPROM is invalid. Size %d", size);
			return;
		}
		else
		{
			std::map<String, LightMapValue>::iterator it;
			int8_t i = 0;
			for (it = mTimeOfLight.begin(); it != mTimeOfLight.end(); it++)
			{
				it->second.first.sw 		= value[i++];
				it->second.first.hour 		= value[i++];
				it->second.first.minute 	= value[i++];
				it->second.first.second 	= value[i++];
				it->second.second.sw 		= value[i++];
				it->second.second.hour 		= value[i++];
				it->second.second.minute 	= value[i++];
				it->second.second.second 	= value[i++];

				LOGD("TIME ON:  %s  %d-%d:%d:%d", it->first,
											 	  it->second.first.sw,
											 	  it->second.first.hour,
											 	  it->second.first.minute,
												  it->second.first.second);
				LOGD("TIME OFF: %s  %d-%d:%d:%d", it->first,
											 	  it->second.first.sw,
											 	  it->second.first.hour,
											 	  it->second.first.minute,
												  it->second.first.second);
			}
		}
	}
}