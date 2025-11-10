#ifndef RTCDEF_H
#define RTCDEF_H

#include <Arduino.h>
#include "Hardware.h"

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
		ON_SWITCH = 0x48U,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
		END_OF_LIGHT1,
	};
}

namespace LIGHT2
{
	enum : uint8_t
	{
		ON_SWITCH = LIGHT1::END_OF_LIGHT1,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
		END_OF_LIGHT2,
	};
}

namespace LIGHT3
{
	enum : uint8_t
	{
		ON_SWITCH = LIGHT2::END_OF_LIGHT2,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
		END_OF_LIGHT3,
	};
}

namespace LIGHT4
{
	enum : uint8_t
	{
		ON_SWITCH = LIGHT3::END_OF_LIGHT3,
		ON_HOUR,
		ON_MINUTE,
		ON_SECOND,
		OFF_SWITCH,
		OFF_HOUR,
		OFF_MINUTE,
		OFF_SECOND,
		END_OF_LIGHT4,
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

struct LightOfTime
{
	uint8_t sw;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};
#endif // RTCDEF_H