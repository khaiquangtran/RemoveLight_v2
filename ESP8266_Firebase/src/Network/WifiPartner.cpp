#include "./WifiPartner.h"

WifiPartner::WifiPartner(Processor *processor) : mProcessor(processor)
{
    mSSID = WIFI_SSID;
    mPassword = WIFI_PASSWORD;
    WiFi.begin(mSSID, mPassword);
    mConfig.api_key = API_KEY;
    mConfig.database_url = DATABASE_URL;

    mCommandAllTimerFlag = -1;

    mTimeClient = new NTPClient(ntpUDP, "pool.ntp.org");
    mTimeClient->begin();
    mTimeClient->setTimeOffset(GMT);
}

void WifiPartner::handleSignal(const SignalType signal, Package *data)
{
    switch (signal)
    {
    case SignalType::STATUS_WIFI:
    {
        connectWifi();
        break;
    }
    case SignalType::STATUS_FIREBASE:
    {
        signUp();
        break;
    }
    case SignalType::STATUS_NTP:
    {
        checkConnectNTP();
        break;
    }
    case SignalType::CHECK_COMMAND_FIREBASE:
    {
        checkCommandFirebase();
        break;
    }
    case SignalType::WEB_GET_ALLTIME_DATA_RESPONSE:
    {
        sendAllTimeDatatoWeb(data);
        break;
    }
    case SignalType::WEB_SET_ALLTIME_DATA_RESPONSE:
    {
        sendResponseSetAllTimeDatatoWeb();
        break;
    }
    default:
        break;
    }
}

void WifiPartner::connectWifi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        mProcessor->handleSignal(SignalType::CONNECT_WIFI_FAILED);
    }
    else
    {
        mProcessor->handleSignal(SignalType::CONNECT_WIFI_SUCCESSFULL);
    }
}

void WifiPartner::signUp()
{
    // Serial.println("Connecting to Firebase...");
    if (Firebase.signUp(&mConfig, &mAuth, "", "") == true)
    {
        Firebase.begin(&mConfig, &mAuth);
        // Firebase.reconnectWiFi(true);
        if(Firebase.ready()) {
            mProcessor->handleSignal(SignalType::CONNECT_FIREBASE_SUCCESSFULL);
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
        }
    }
    else
    {
        mProcessor->handleSignal(SignalType::CONNECT_FIREBASE_FAILED);
    }
}

void WifiPartner::checkConnectNTP()
{
    if (mTimeClient->getEpochTime() > 0)
    {
        mProcessor->handleSignal(SignalType::CONNECT_NTP_SUCCESSFULL);
    }
    else
    {
        mProcessor->handleSignal(SignalType::CONNECT_NTP_FAILED);
    }
}

void WifiPartner::checkCommandFirebase()
{
    if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_COMMAND_PATH))
    {
        if (mFbdo.intData() == 1)
        {
            Firebase.RTDB.setInt(&mFbdo, ALLTIME_COMMAND_PATH, 3);
            mProcessor->handleSignal(SignalType::WEB_GET_ALLTIME_DATA_REQUEST);
        }
        else if(mFbdo.intData() == 2)
        {
            Firebase.RTDB.setInt(&mFbdo, ALLTIME_COMMAND_PATH, 3);
            // Format package: second minute hour day date month year
            //                 xx     xx     xx   xx  xx   xx    xxxx
            int second  = 0;
            int minute  = 0;
            int hour    = 0;
            int day     = 0;
            int date    = 0;
            int month   = 0;
            int year    = 0;

            if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_DATA_SECOND_PATH)) {
                if (mFbdo.dataType() == "int") {
                    second = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
                return;
            }

            if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_DATA_MINUTE_PATH)) {
                if (mFbdo.dataType() == "int") {
                    minute = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
                return;
            }

            if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_DATA_HOUR_PATH)) {
                if (mFbdo.dataType() == "int") {
                    hour = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
                return;
            }

            if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_DATA_DAY_PATH)) {
                if (mFbdo.dataType() == "int") {
                    day = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
                return;
            }

            if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_DATA_DATE_PATH)) {
                if (mFbdo.dataType() == "int") {
                    date = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
                return;
            }

            if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_DATA_MONTH_PATH)) {
                if (mFbdo.dataType() == "int") {
                    month = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
                return;
            }

            if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_DATA_YEAR_PATH)) {
                if (mFbdo.dataType() == "int") {
                    year = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
                return;
            }
            Serial.println(second);
            Serial.println(minute);
            Serial.println(hour);
            Serial.println(day);
            Serial.println(date);
            Serial.println(month);
            Serial.println(year);
            int data[7] = {second, minute, hour, day, date, month, year};
            Package *package = new Package(data, 7);
            mProcessor->handleSignal(SignalType::WEB_SET_ALLTIME_DATA_REQUEST, package);
            delete package;
        }
    }

    else
    {
        Serial.print("error: ");
        Serial.println(mFbdo.errorReason());
    }
}

void WifiPartner::sendAllTimeDatatoWeb(Package *data)
{
    int *parseData  = data->getPackage();
    if(data->getSize() == 8)
    {
        int second      = parseData[1];
        int minute      = parseData[2];
        int hour        = parseData[3];
        int day         = parseData[4];
        int date        = parseData[5];
        int month       = parseData[6];
        int year        = parseData[7];
        if (Firebase.RTDB.setInt(&mFbdo, ALLTIME_DATA_SECOND_PATH, second)) {
            Serial.println("Sended SECOND DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }
        if(Firebase.RTDB.setInt(&mFbdo, ALLTIME_DATA_MINUTE_PATH, minute)) {
            Serial.println("Sended MINUTE DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }
        if(Firebase.RTDB.setInt(&mFbdo, ALLTIME_DATA_HOUR_PATH, hour)) {
            Serial.println("Sended HOUR DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }
        if(Firebase.RTDB.setInt(&mFbdo, ALLTIME_DATA_DAY_PATH, day)) {
            Serial.println("Sended DAY DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }
        if(Firebase.RTDB.setInt(&mFbdo, ALLTIME_DATA_DATE_PATH, date)) {
            Serial.println("Sended DATE DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }
        if(Firebase.RTDB.setInt(&mFbdo, ALLTIME_DATA_MONTH_PATH, month)) {
            Serial.println("Sended MONTH DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }
        if(Firebase.RTDB.setInt(&mFbdo, ALLTIME_DATA_YEAR_PATH, year)) {
            Serial.println("Sended YEAR DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }

        if (Firebase.RTDB.setInt(&mFbdo, ALLTIME_COMMAND_PATH, 0)) {
            Serial.println("Sended ALLTIME COMMAND DATA");
        }
        else {
            Serial.println("FAILED");
            Serial.println("REASON: " + mFbdo.errorReason());
            return;
        }
    }
    else {
        Serial.println("sendAllTimeDatatoWeb(): Length is invalid");
    }
}

void WifiPartner::sendResponseSetAllTimeDatatoWeb()
{
    if (Firebase.RTDB.setInt(&mFbdo, ALLTIME_COMMAND_PATH, 0)) {
        Serial.println("Sended ALL TIME DATA response");
    }
    else {
        Serial.println("FAILED");
        Serial.println("REASON: " + mFbdo.errorReason());
        return;
    }
}