#include "./Hardware.h"

uint8_t Hardware::scanAddress(uint8_t addressInput)
{
	uint8_t error, address;

	LOGI("Scanning address …");

	for (address = 1; address < 127; address++)
	{
		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		if (error == 0)
		{
			if (address == addressInput)
			{
				LOGD("I2C found at address 0x%x", address);
				return address;
				break;
			}
		}
		else if (error == 4)
		{
			LOGE("Unknow error at address 0x%x", address);
			return INVALID;
		}
	}
	LOGD("Scanning done!!!");
	return INVALID;
}

int *Hardware::parseCommandStringToArray(String str, int &size)
{
    char cstr[100];
    str.toCharArray(cstr, 50);

    char *pch;
    int *pnum = new int[10];
    int index = 0;
    pch = strtok(cstr, " ");
    while (pch != NULL)
    {
      pnum[index++] = atoi(pch);
      pch = strtok(NULL, " ");
    }
    size = index;
    return pnum;
}