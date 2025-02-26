#ifndef RTC_H
#define RTC_H

#include "./Hardware.h"
#include "./RTCDef.h"
#include "./../RemoteLight.h"

using TimePair = std::pair<REG_TIME_LIGHT, TimeOfLight>;
using LightMapValue = std::pair<TimePair, TimePair>;

class RemoteLight;

class RTC : public Hardware
{
public:
	RTC(RemoteLight *rml);
	~RTC();
	RTC(const RTC &) = delete;
	RTC &operator=(const RTC &) = delete;

	void handleSignal(const SignaLType signal, Package *data = nullptr);

private:
	RemoteLight *mRML;
	bool checkAddress();
	byte bcdToDec(byte val);
	byte decToHex(byte val);
	void writeData(uint8_t reg, uint8_t data);
	struct TimeDS1307 getTimeData();
	bool setTimeData(struct TimeDS1307 data);
	struct TimeOfLight getTimeLight(String light, uint8_t reg);
	void setTimeLight(String light, struct TimeOfLight time, struct REG_TIME_LIGHT reg);

	void getResponse(struct TimeOfLight *time, uint8_t REG);
	void sendAllTimeData(const SignaLType signal);
	void increaseValueOfTimeData();
	void decreaseValueOfTimeData();
	void shiftIndexOfAllTimeData(bool adjust);
	void increaseValueOfMenuMode();
	void decreaseValueOfMenuMode();
	void sendTimeOfLight();
	void sendAllTimeDataToWeb();
	struct TimeOfLight getTimeOfLight(uint8_t reg);
	void checkConfigureTimeForLight();
	void requestSetTimeAllData(Package *data);

	uint8_t mDS1307Addr;
	std::map<String, LightMapValue> mTimeOfLight;
	const uint8_t DS1307_ADDR = 0x68;
	struct TimeDS1307 mAllTimeData;
	uint8_t mIndexOfAllTimeData;
	const String LISTLIGHT[4] = {"Light1", "Light2", "Light3", "Light4"};
	uint8_t mIndexListLight;
	uint8_t mIndexLight;
	uint8_t mCountRetry;
	const uint8_t RETRY = 3U;
};

#endif // RTC_H