#ifndef RTC_H
#define RTC_H

#include "./Hardware.h"
#include "./RTCDef.h"
#include "./../RemoteLight.h"

// 				the time the light turns on - turns off
using LightMapValue = std::pair<LightOfTime, LightOfTime>;

class RemoteLight;

class RTC : public Hardware
{
public:
	RTC(std::shared_ptr<RemoteLight> rml);
	~RTC();
	RTC(const RTC &) = delete;
	RTC &operator=(const RTC &) = delete;

	void handleSignal(const SignalType& signal, const Package* data = nullptr);
	void init();

private:
	std::shared_ptr<RemoteLight> mRML;
	bool checkAddress();
	byte bcdToDec(byte val);
	byte decToHex(byte val);
	bool writeData(uint8_t reg, uint8_t data);
	struct TimeDS1307 getTimeData();
	bool setTimeData(struct TimeDS1307 data);

	void sendAllTimeData(const SignalType signal);
	void increaseValueOfTimeData();
	void decreaseValueOfTimeData();
	void shiftIndexOfAllTimeData(bool adjust);
	void increaseValueOfMenuMode();
	void decreaseValueOfMenuMode();
	void sendTimeOfLight();
	void sendAllTimeDataToWeb();
	void checkConfigureTimeForLight();
	void requestSetTimeAllData(const Package* data);
	void sendLightDataToWeb(const SignalType signal);
	void requestSetLightData(const Package* data);
	void receiveTimeDateFromNTP(const Package *data);
	void adjustTime();
	void updateTimeForRTC();
	void parseTimeDataFromEEPROM(const Package* data);
	void getLightOnOffDataFromEEPROM(const Package* data);

	uint8_t mRTCAddr;
	std::map<String, LightMapValue> mTimeOfLight;
	const uint8_t RTC_ADDRESS = 0x68;
	struct TimeDS1307 mAllTimeData;
	uint8_t mIndexOfAllTimeData;
	const String LISTLIGHT[4] = {"Light1", "Light2", "Light3", "Light4"};
	uint8_t mIndexListLight;
	uint8_t mIndexLight;
	uint8_t mCountRetry;
	const uint8_t RETRY = 3U;
	std::map<SignalType, std::pair<int32_t, SignalType>> mLightGetRequestResponse;
	std::map<SignalType, std::pair<int32_t, SignalType>> mLightSetRequestResponse;

	uint8_t mFlagUpdateTIme;
	uint8_t mPreDate;
	uint8_t mCounterUpdateTime;

	struct LightOfTime mTimeTemp;
};

#endif // RTC_H