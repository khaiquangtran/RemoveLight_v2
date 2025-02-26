#include "./SerialPartner.h"

SerialPartner::SerialPartner(Processor *processor) : mProcessor(processor)
{
    Serial.begin(BAUD_RATE);
    mSerial2 = new SoftwareSerial(RXD2, TXD2);
    mSerial2->begin(BAUD_RATE);

    mCommandHandle[COMMAND::STATUS_WIFI] = "0001";
    mCommandHandle[COMMAND::STATUS_FIREBASE] = "0002";
    mCommandHandle[COMMAND::STATUS_NTP] = "0003";
    mCommandHandle[COMMAND::WIFI_SUCCESSFULL] = "1001";
    mCommandHandle[COMMAND::WIFI_FAILED] = "1002";
    mCommandHandle[COMMAND::FIREBASE_SUCCESSFULL] = "2002";
    mCommandHandle[COMMAND::FIREBASE_FAILED] = "2003";
    mCommandHandle[COMMAND::NTP_SUCCESSFULL] = "3002";
    mCommandHandle[COMMAND::NTP_FAILED] = "3003";
    mCommandHandle[COMMAND::WEB_GET_ALLTIME_DATA_REQUEST] = "4004";
    mCommandHandle[COMMAND::WEB_GET_ALLTIME_DATA_RESPONSE] = "4005";
}

SerialPartner::~SerialPartner()
{
}

void SerialPartner::listen()
{
    String receiverData;
    if (Serial.available())
    {
        String c = Serial.readString();
        mSerial2->print(c);
    }

    if (mSerial2->available())
    {
        String c = mSerial2->readString();
        Serial.println(c.c_str());
        handleMessage(c);
    }
}

void SerialPartner::handleSignal(const SignalType signal, Package *data)
{
    switch (signal)
    {
    case SignalType::CONNECT_WIFI_SUCCESSFULL:
    {
        mSerial2->write(mCommandHandle[COMMAND::WIFI_SUCCESSFULL].c_str());
        break;
    }
    case SignalType::CONNECT_FIREBASE_SUCCESSFULL:
    {
        mSerial2->write(mCommandHandle[COMMAND::FIREBASE_SUCCESSFULL].c_str());
        break;
    }
    case SignalType::CONNECT_NTP_SUCCESSFULL:
    {
        mSerial2->write(mCommandHandle[COMMAND::NTP_SUCCESSFULL].c_str());
        break;
    }
    case SignalType::CONNECT_WIFI_FAILED:
    {
        mSerial2->write(mCommandHandle[COMMAND::WIFI_FAILED].c_str());
        break;
    }
    case SignalType::CONNECT_FIREBASE_FAILED:
    {
        mSerial2->write(mCommandHandle[COMMAND::FIREBASE_FAILED].c_str());
        break;
    }
    case SignalType::CONNECT_NTP_FAILED:
    {
        mSerial2->write(mCommandHandle[COMMAND::NTP_FAILED].c_str());
        break;
    }
    case SignalType::WEB_GET_ALLTIME_DATA_REQUEST:
    {
        mSerial2->write(mCommandHandle[COMMAND::WEB_GET_ALLTIME_DATA_REQUEST].c_str());
        break;
    }
    default:
        break;
    }
}

void SerialPartner::handleMessage(String receiverData)
{
    String command = receiverData.substring(0, 4);
    if (command == mCommandHandle[COMMAND::STATUS_WIFI])
    {
        mProcessor->handleSignal(SignalType::STATUS_WIFI);
    }
    else if (command == mCommandHandle[COMMAND::STATUS_FIREBASE])
    {
        mProcessor->handleSignal(SignalType::STATUS_FIREBASE);
    }
    else if (command == mCommandHandle[COMMAND::STATUS_NTP])
    {
        mProcessor->handleSignal(SignalType::STATUS_NTP);
    }
    else if (command == mCommandHandle[COMMAND::WEB_GET_ALLTIME_DATA_RESPONSE])
    {
        int size = 0;
        int *data = parseCommandStringToArray(receiverData, size);
        Package *package = new Package(data, size);
        mProcessor->handleSignal(SignalType::WEB_GET_ALLTIME_DATA_RESPONSE, package);
    }
}

int *SerialPartner::parseCommandStringToArray(String str, int &size)
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