#include "./WifiPartner.h"

WifiPartner::WifiPartner(Processor *processor) : mProcessor(processor)
{
    mSSID = WIFI_SSID;
    mPassword = WIFI_PASSWORD;
    WiFi.begin(mSSID, mPassword);
    mConfig.api_key = API_KEY;
    mConfig.database_url = DATABASE_URL;

    mCommandAllTimerFlag = -1;

    mSignalLightMap = {
        {SignalType::WEB_GET_LIGHT1_DATA_RESPONSE, 1},
        {SignalType::WEB_GET_LIGHT2_DATA_RESPONSE, 2},
        {SignalType::WEB_GET_LIGHT3_DATA_RESPONSE, 3},
        {SignalType::WEB_GET_LIGHT4_DATA_RESPONSE, 4},
    };

    mRequestSignalMap = {
        {REQUEST_FB::GETTING_LIGHT1_DATA, std::make_pair(SignalType::WEB_GET_LIGHT1_DATA_REQUEST, 1)},
        {REQUEST_FB::GETTING_LIGHT2_DATA, std::make_pair(SignalType::WEB_GET_LIGHT2_DATA_REQUEST, 1)},
        {REQUEST_FB::GETTING_LIGHT3_DATA, std::make_pair(SignalType::WEB_GET_LIGHT3_DATA_REQUEST, 1)},
        {REQUEST_FB::GETTING_LIGHT4_DATA, std::make_pair(SignalType::WEB_GET_LIGHT4_DATA_REQUEST, 1)},

        {REQUEST_FB::SETTING_LIGHT1_DATA, std::make_pair(SignalType::WEB_SET_LIGHT1_DATA_REQUEST, 1)},
        {REQUEST_FB::SETTING_LIGHT2_DATA, std::make_pair(SignalType::WEB_SET_LIGHT2_DATA_REQUEST, 2)},
        {REQUEST_FB::SETTING_LIGHT3_DATA, std::make_pair(SignalType::WEB_SET_LIGHT3_DATA_REQUEST, 3)},
        {REQUEST_FB::SETTING_LIGHT4_DATA, std::make_pair(SignalType::WEB_SET_LIGHT4_DATA_REQUEST, 4)},

        {REQUEST_FB::SETTING_LIGHT1_STATUS, std::make_pair(SignalType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST, 9)},
        {REQUEST_FB::SETTING_LIGHT2_STATUS, std::make_pair(SignalType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST, 10)},
        {REQUEST_FB::SETTING_LIGHT3_STATUS, std::make_pair(SignalType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST, 11)},
        {REQUEST_FB::SETTING_LIGHT4_STATUS, std::make_pair(SignalType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST, 12)},
    };

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
    case SignalType::WEB_GET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT4_DATA_RESPONSE:
    {
        sendLightDataToWeb(data, mSignalLightMap[signal]);
        break;
    }
    case SignalType::WEB_SET_ALLTIME_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT4_DATA_RESPONSE:
    case SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE:
    {
        sendResponseSetLightDatatoWeb();
        break;
    }
    case SignalType::WEB_GET_STATUS_DATA_RESPONSE:
    {
        sendLightStatusToWeb(data);
        break;
    }
    case SignalType::REMOTE_LIGHT_GET_TIME_DATE_FROM_NTP:
    {
        getTimeDataFromNtp();
        break;
    }
    default:
        break;
    }
}

void WifiPartner::connectWifi()
{
    if (WiFi.status() != WL_CONNECTED) {
        mProcessor->handleSignal(SignalType::CONNECT_WIFI_FAILED);
    }
    else {
        mProcessor->handleSignal(SignalType::CONNECT_WIFI_SUCCESSFULL);
    }
}

void WifiPartner::signUp()
{
    // Serial.println("Connecting to Firebase...");
    if (Firebase.signUp(&mConfig, &mAuth, "", "") == true) {
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
    else {
        mProcessor->handleSignal(SignalType::CONNECT_FIREBASE_FAILED);
    }
}

void WifiPartner::checkConnectNTP()
{
    if (mTimeClient->getEpochTime() > 0) {
        mProcessor->handleSignal(SignalType::CONNECT_NTP_SUCCESSFULL);
    }
    else {
        mProcessor->handleSignal(SignalType::CONNECT_NTP_FAILED);
    }
}

void WifiPartner::checkCommandFirebase()
{
    if (Firebase.RTDB.getInt(&mFbdo, DATA_PATHS.at(0))) {
        REQUEST_FB typeCommand = static_cast<REQUEST_FB>(mFbdo.intData());
        switch (typeCommand)
        {
        case REQUEST_FB::GETTING_ALLTIME_DATA:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            mProcessor->handleSignal(SignalType::WEB_GET_ALLTIME_DATA_REQUEST);
            break;
        }
        case REQUEST_FB::SETTING_ALLTIME_DATA:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            int size = 7;
            // Format:        hour minute second day date month year
            int data[size] = {0U, 0U, 0U, 0U, 0U, 0U, 0U};
            for(int i = 1; i < 8; i++) {
                if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_PATH.at(i))) {
                    if (mFbdo.dataType() == "int") {
                        data[i-1] = mFbdo.intData();
                    }
                }
                else {
                    Serial.println(mFbdo.errorReason());
                    return;
                }
            }
            Package *package = new Package(data, size);
            mProcessor->handleSignal(SignalType::WEB_SET_ALLTIME_DATA_REQUEST, package);
            delete package;
            break;
        }
        case REQUEST_FB::GETTING_LIGHT1_DATA:
        case REQUEST_FB::GETTING_LIGHT2_DATA:
        case REQUEST_FB::GETTING_LIGHT3_DATA:
        case REQUEST_FB::GETTING_LIGHT4_DATA:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            mProcessor->handleSignal(mRequestSignalMap[typeCommand].first);
            break;
        }
        case REQUEST_FB::SETTING_LIGHT1_DATA:
        case REQUEST_FB::SETTING_LIGHT2_DATA:
        case REQUEST_FB::SETTING_LIGHT3_DATA:
        case REQUEST_FB::SETTING_LIGHT4_DATA:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            // Format: swOn hourOn minuteOn secondOn swOff hourOff minuteOff secondOff
            int data[8] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
            String path;
            for(int i = 1; i < 9; i++)
            {
                path = LIGHT_PATHS.at(mRequestSignalMap[typeCommand].second) + DATA_PATHS.at(i);
                if (Firebase.RTDB.getInt(&mFbdo, path.c_str())) {
                    if (mFbdo.dataType() == "int") {
                        data[i-1] = mFbdo.intData();
                    }
                }
                else {
                    Serial.println(mFbdo.errorReason());
                    return;
                }
            }

            Package *package = new Package(data, 8);
            mProcessor->handleSignal(mRequestSignalMap[typeCommand].first, package);
            delete package;
            break;
        }
        case REQUEST_FB::GETTING_ALL_STATUS:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            mProcessor->handleSignal(SignalType::WEB_GET_STATUS_DATA_REQUEST);
            break;
        }
        case REQUEST_FB::SETTING_LIGHT1_STATUS:
        case REQUEST_FB::SETTING_LIGHT2_STATUS:
        case REQUEST_FB::SETTING_LIGHT3_STATUS:
        case REQUEST_FB::SETTING_LIGHT4_STATUS:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            int data = {0U};

            String path = LIGHT_PATHS.at(5) + DATA_PATHS.at(mRequestSignalMap[typeCommand].second);
            if (Firebase.RTDB.getInt(&mFbdo, path.c_str())) {
                if (mFbdo.dataType() == "int") {
                    data = mFbdo.intData();
                }
            }
            else {
                Serial.println(mFbdo.errorReason());
            }
            Package *package = new Package(&data, 1);
            mProcessor->handleSignal(mRequestSignalMap[typeCommand].first, package);
            delete package;
            break;
        }
        default:
            break;
        }
    }
    else {
        Serial.print("error: ");
        Serial.println(mFbdo.errorReason());
        return;
    }
}

void WifiPartner::sendAllTimeDatatoWeb(Package *data)
{
    if(data->getSize() == 8U) {
        int *parseData  = data->getPackage();
        for(int i = 1; i < 8; i++)
        {
            if (Firebase.RTDB.setInt(&mFbdo, ALLTIME_PATH.at(i), parseData[i])) {
                Serial.println("Sent SECOND DATA");
            }
            else {
                Serial.println("FAILED");
                Serial.println("REASON: " + mFbdo.errorReason());
                return;
            }
        }

        // Inform to Firebase server
        if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
            Serial.println("Sent ALLTIME COMMAND DATA");
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

void WifiPartner::sendLightDataToWeb(Package *data, int lightIndex)
{
    if(data->getSize() == 9U) {
        // ---------Format---------
        // swOn      = parseData[1]
        // hourOn    = parseData[2]
        // minuteOn  = parseData[3]
        // secondOn  = parseData[4]
        // swOff     = parseData[5]
        // hourOff   = parseData[6]
        // minuteOff = parseData[7]
        // secondOff = parseData[8]

        String path;
        int *parseData  = data->getPackage();

        for(int i = 1; i < 9; i++)
        {
            path = LIGHT_PATHS.at(lightIndex) + DATA_PATHS.at(i);
            if (Firebase.RTDB.setInt(&mFbdo, path.c_str(), parseData[i])) {
                Serial.println("Sent DATA");
            }
            else {
                Serial.println("FAILED: " + mFbdo.errorReason());
                return;
            }
        }

        if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
            Serial.println("Sent LIGHT COMMAND DATA");
        }
        else {
            Serial.println("FAILED: " + mFbdo.errorReason());
            return;
        }
    }
    else {
        Serial.println("sendLightDataToWeb(): Length is invalid");
    }
}

void WifiPartner::sendResponseSetLightDatatoWeb()
{
    if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
        Serial.println("Sent LIGHT response");
    }
    else {
        Serial.println("FAILED: " + mFbdo.errorReason());
    }
}

void WifiPartner::sendLightStatusToWeb(Package *data)
{
    if(data->getSize() == 5U) {
        String path;
        int *parseData  = data->getPackage();
        for(int i = 1; i < 5; i++)
        {
            path = LIGHT_PATHS.at(5) + DATA_PATHS.at(i + 8);
            if (Firebase.RTDB.setInt(&mFbdo, path.c_str(), parseData[i])) {
                Serial.println("Sent STATUS");
            }
            else {
                Serial.println("FAILED: " + mFbdo.errorReason());
                return;
            }
        }

        if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
            Serial.println("Sent STATUS COMMAND DATA");
        }
        else {
            Serial.println("FAILED: " + mFbdo.errorReason());
        }
    }
    else {
        Serial.println("sendLightStatusToWeb(): Length is invalid");
    }
}

void WifiPartner::getTimeDataFromNtp()
{
    const int size = 7U;
    // Format:        hour minute second day date month year
    int data[size] = {0U, 0U, 0U, 0U, 0U, 0U, 0U};

    mTimeClient->update();
    time_t epochTime = mTimeClient->getEpochTime();
    if (epochTime > 0) {
        struct tm *ptm = gmtime ((time_t *)&epochTime);
        data[0] = ptm->tm_hour;
        data[1] = ptm->tm_min;
        data[2] = ptm->tm_sec;
        data[3] = ptm->tm_wday + 1;
        data[4] = ptm->tm_mday;
        data[5] = ptm->tm_mon + 1; // tm_mon is 0-based
        data[6] = ptm->tm_year + 1900; // tm_year is years since 1900
    }

    Package *package = new Package(data, size);
    mProcessor->handleSignal(SignalType::REMOTE_LIGHT_SEND_TIME_DATE_FROM_NTP, package);
    delete package;
}