#include "./Network.h"

Network::Network(std::shared_ptr<RemoteLight>rml) : mRML(rml)
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

void Network::handleSignal(const SignalType signal, Package *data)
{
    switch (signal)
    {
    case SignalType::NETWORK_CHECK_STATUS_WIFI:
    {
        connectWifi();
        break;
    }
    case SignalType::NETWORK_CHECK_STATUS_FIREBASE:
    {
        signUp();
        break;
    }
    case SignalType::NETWORK_CHECK_STATUS_NTP:
    {
        checkConnectNTP();
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
    case SignalType::NETWORK_GET_TIME_DATE_FROM_NTP:
    {
        getTimeDataFromNtp();
        break;
    }
    case SignalType::REMOTE_LIGHT_CHECK_COMMAND_FIREBASE:
    {
        checkCommandFirebase();
        break;
    }
    default:
        break;
    }
}

void Network::connectWifi()
{
    if (WiFi.status() != WL_CONNECTED) {
        mRML->handleSignal(SignalType::TASKS_CONNECT_WIFI_FAILED);
    }
    else {
        mRML->handleSignal(SignalType::TASKS_CONNECT_WIFI_SUCCESS);
    }
}

void Network::signUp()
{
    // Serial.println("Connecting to Firebase...");
    if (Firebase.signUp(&mConfig, &mAuth, "", "") == true) {
        Firebase.begin(&mConfig, &mAuth);
        // Firebase.reconnectWiFi(true);
        if(Firebase.ready()) {
            mRML->handleSignal(SignalType::TASKS_CONNECT_FIREBASE_SUCCESS);
        }
        else {
            LOGE("REASON: %s", mFbdo.errorReason().c_str());
            mRML->handleSignal(SignalType::TASKS_CONNECT_FIREBASE_FAILED);
        }
    }
    else {
        LOGE("REASON: %s", mFbdo.errorReason().c_str());
        mRML->handleSignal(SignalType::TASKS_CONNECT_FIREBASE_FAILED);
    }
}

void Network::checkConnectNTP()
{
    if (mTimeClient->getEpochTime() > 0) {
        mRML->handleSignal(SignalType::TASKS_CONNECT_NTP_SUCCESS);
    }
    else {
        mRML->handleSignal(SignalType::TASKS_CONNECT_NTP_FAILED);
        LOGE("NTP server connect failed");
    }
}

void Network::checkCommandFirebase()
{
    if (Firebase.RTDB.getInt(&mFbdo, DATA_PATHS.at(0))) {
        REQUEST_FB typeCommand = static_cast<REQUEST_FB>(mFbdo.intData());
        switch (typeCommand)
        {
        case REQUEST_FB::GETTING_ALLTIME_DATA:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            mRML->handleSignal(SignalType::WEB_GET_ALLTIME_DATA_REQUEST);
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
                    LOGE("REASON: %s", mFbdo.errorReason().c_str());
                    return;
                }
            }
            Package *package = new Package(data, size);
            mRML->handleSignal(SignalType::WEB_SET_ALLTIME_DATA_REQUEST, package);
            delete package;
            break;
        }
        case REQUEST_FB::GETTING_LIGHT1_DATA:
        case REQUEST_FB::GETTING_LIGHT2_DATA:
        case REQUEST_FB::GETTING_LIGHT3_DATA:
        case REQUEST_FB::GETTING_LIGHT4_DATA:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            mRML->handleSignal(mRequestSignalMap[typeCommand].first);
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
                    LOGE("REASON: %s", mFbdo.errorReason().c_str());
                    return;
                }
            }

            Package *package = new Package(data, 8);
            mRML->handleSignal(mRequestSignalMap[typeCommand].first, package);
            delete package;
            break;
        }
        case REQUEST_FB::GETTING_ALL_STATUS:
        {
            Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(0), static_cast<int>(REQUEST_FB::IDLE));
            mRML->handleSignal(SignalType::WEB_GET_STATUS_DATA_REQUEST);
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
                LOGE("REASON: %s", mFbdo.errorReason().c_str());
            }
            Package *package = new Package(&data, 1);
            mRML->handleSignal(mRequestSignalMap[typeCommand].first, package);
            delete package;
            break;
        }
        default:
            break;
        }
    }
    else {
        LOGE("REASON: %s", mFbdo.errorReason().c_str());
        // mRML->handleSignal(SignalType::CONNECT_RETRY);
        return;
    }
}

void Network::sendAllTimeDatatoWeb(Package *data)
{
    const int SIZE_OF_ALLTIME_DATA = 7U;
    if(data->getSize() != SIZE_OF_ALLTIME_DATA) {
        LOGE("sendAllTimeDatatoWeb(): Length is invalid");
    }
    else {
        const int INDEX_COMMAND_PATH = 0U;
        const int START_INDEX_ALLTIME_PATH = 1U;

        int *parseData  = data->getPackage();
        for(int i = 0; i < SIZE_OF_ALLTIME_DATA; i++)
        {
            if (Firebase.RTDB.setInt(&mFbdo, ALLTIME_PATH.at(i + START_INDEX_ALLTIME_PATH), parseData[i])) {
                LOGI("Sent SECOND DATA");
            }
            else {
                LOGE("REASON: %s", mFbdo.errorReason().c_str());
                return;
            }
        }

        // Inform to Firebase server
        if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(INDEX_COMMAND_PATH), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
            LOGI("Sent ALLTIME COMMAND DATA");
        }
        else {
            LOGE("REASON: %s", mFbdo.errorReason().c_str());
        }
    }
}

void Network::sendLightDataToWeb(Package *data, int lightIndex)
{
    const int SIZE_OF_LIGHT_DATA = 8U;
    if(data->getSize() != SIZE_OF_LIGHT_DATA) {
        LOGI("sendLightDataToWeb(): Length is invalid");
    }
    else {
        // ---------Format---------
        // swOn      = parseData[0]
        // hourOn    = parseData[1]
        // minuteOn  = parseData[2]
        // secondOn  = parseData[3]
        // swOff     = parseData[4]
        // hourOff   = parseData[5]
        // minuteOff = parseData[6]
        // secondOff = parseData[7]

        String path;
        const int INDEX_COMMAND_PATH = 0U;
        const int START_INDEX_LIGHT_PATH = 1U;

        int *parseData  = data->getPackage();

        for(int i = 0; i < 8; i++)
        {
            path = LIGHT_PATHS.at(lightIndex) + DATA_PATHS.at(i + START_INDEX_LIGHT_PATH);
            if (Firebase.RTDB.setInt(&mFbdo, path.c_str(), parseData[i])) {
                LOGI("Sent DATA");
            }
            else {
                LOGE("REASON: %s", mFbdo.errorReason().c_str());
                return;
            }
        }

        if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(INDEX_COMMAND_PATH), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
            LOGI("Sent LIGHT COMMAND DATA");
        }
        else {
            LOGE("REASON: %s", mFbdo.errorReason().c_str());
            return;
        }
    }
}

void Network::sendResponseSetLightDatatoWeb()
{
    const int INDEX_COMMAND_PATH = 0U;
    if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(INDEX_COMMAND_PATH), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
        LOGI("Sent LIGHT response");
    }
    else {
        LOGE("FAILED: ", mFbdo.errorReason().c_str());
    }
}

void Network::sendLightStatusToWeb(Package *data)
{
    const int SIZE_OF_LIGHT_STATUS = 4U;
    if(data->getSize() != SIZE_OF_LIGHT_STATUS) {
        LOGE("sendLightStatusToWeb(): Length is invalid");
    }
    else {
        String path;
        const int INDEX_STATUS_PATH = 5U;
        const int START_INDEX_STATUS_PATH = 9U;
        const int INDEX_COMMAND_PATH = 0U;
        int *parseData  = data->getPackage();
        for(int i = 0; i < SIZE_OF_LIGHT_STATUS; i++)
        {
            path = LIGHT_PATHS.at(INDEX_STATUS_PATH) + DATA_PATHS.at(i + START_INDEX_STATUS_PATH);
            if (Firebase.RTDB.setInt(&mFbdo, path.c_str(), parseData[i])) {
                LOGI("Sent STATUS");
            }
            else {
                LOGE("FAILED: ", mFbdo.errorReason().c_str());
                return;
            }
        }

        if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(INDEX_COMMAND_PATH), static_cast<int>(REQUEST_FB::SENT_INFORM))) {
            LOGI("Sent STATUS COMMAND DATA");
        }
        else {
            LOGE("FAILED: ", mFbdo.errorReason().c_str());
        }
    }
}

void Network::getTimeDataFromNtp()
{
    LOGI(".");
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
    mRML->handleSignal(SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP, package);
    delete package;
}