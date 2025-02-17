#ifndef RTCDEF_H
#define RTCDEF_H

#include <Arduino.h>

enum REGISTER_ADDRESS : uint8_t
{
	REG_SEC = 0x00,
	REG_MIN,
	REG_HOUR,
	REG_DAY,
	REG_DATE,
	REG_MTH,
	REG_YEAR,
};

namespace LIGHT1
{
	enum : uint8_t
	{
		ON_SWITCH = 0x08,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
	};
}

namespace LIGHT2
{
	enum : uint8_t
	{
		ON_SWITCH = LIGHT1::OFF_SECOND,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
	};
}

namespace LIGHT3
{
	enum : uint8_t
	{
		ON_SWITCH = LIGHT2::OFF_SECOND,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
	};
}

namespace LIGHT4
{
	enum : uint8_t
	{
		ON_SWITCH = LIGHT3::OFF_SECOND,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
	};
}

struct TimeDS1307
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint16_t year;
};

struct TimeOfLight
{
	uint8_t sw;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

struct REG_TIME_LIGHT
{
	uint8_t SWITCH;
	uint8_t HOUR;
	uint8_t MINUTE;
	uint8_t SECOND;
};

#endif // RTCDEF_H