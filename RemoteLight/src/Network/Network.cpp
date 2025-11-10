#include "./Network.h"

Network::Network(std::shared_ptr<RemoteLight>rml) : mRML(rml)
{
    mSSID = WIFI_SSID;
    mPassword = WIFI_PASSWORD;
    mIsConnectedBT = false;
    mIsConnectedWifi = false;
    mPermissionToConnectBT = false;
    // WiFi.begin(mSSID, mPassword);
    // mConfig.api_key = API_KEY;
    // mConfig.database_url = DATABASE_URL;

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

    // mTimeClient = new NTPClient(ntpUDP, "pool.ntp.org");
    // mTimeClient->begin();
    // mTimeClient->setTimeOffset(GMT);
    LOGI(" =========== Network =========== ");
}


void Network::handleSignal(const SignalType signal, Package *data) {
    switch (signal)
    {
    case SignalType::NETWORK_CHECK_STATUS_WIFI:
    {
        connectWifi();
        break;
    }
    case SignalType::NETWORK_CHECK_STATUS_FIREBASE: {
        signUp();
        break;
    }
    case SignalType::NETWORK_CHECK_STATUS_NTP: {
        checkConnectNTP();
        break;
    }
    case SignalType::WEB_GET_ALLTIME_DATA_RESPONSE: {
        sendAllTimeDatatoWeb(data);
        break;
    }
    case SignalType::WEB_GET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT4_DATA_RESPONSE: {
        sendLightDataToWeb(data, mSignalLightMap[signal]);
        break;
    }
    case SignalType::WEB_SET_ALLTIME_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT4_DATA_RESPONSE:
    case SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE: {
        sendResponseSetLightDatatoWeb();
        break;
    }
    case SignalType::WEB_GET_STATUS_DATA_RESPONSE: {
        sendLightStatusToWeb(data);
        break;
    }
    case SignalType::NETWORK_GET_TIME_DATE_FROM_NTP: {
        getTimeDataFromNtp();
        break;
    }
    case SignalType::REMOTE_LIGHT_CHECK_COMMAND_FIREBASE: {
        // checkCommandFirebase();
        break;
    }
    case SignalType::NETWORK_SEND_SSID_PASSWORD: {
        const int32_t* dat = data->getPackage();
        const int32_t size = data->getSize();

        String str = "";
        for (int32_t i =0; i < size; i++) {
            str += static_cast<char>(dat[i]);
        }

        int32_t sep = str.indexOf('%');
        if (sep != -1) {
            mSSID = str.substring(0, sep);
            mPassword = str.substring(sep + 1);

            LOGI("SSID: %s", mSSID);
            LOGI("Password: %s", mPassword);
        }
        break;
    }
    case SignalType::PRESS_BTN_1_2_COMBO_SIGNAL:
    {
        processComboBtnPress();
        break;
    }
    default:
        break;
    }
}

void Network::connectWifi() {
    if (WiFi.status() != WL_CONNECTED) {
        mRML->handleSignal(SignalType::TASKS_CONNECT_WIFI_FAILED);
    }
    else {
        mRML->handleSignal(SignalType::TASKS_CONNECT_WIFI_SUCCESS);
    }
}

void Network::signUp() {
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

void Network::checkConnectNTP() {
    if (mTimeClient->update()) {
        mRML->handleSignal(SignalType::TASKS_CONNECT_NTP_SUCCESS);
    }
    else {
        mRML->handleSignal(SignalType::TASKS_CONNECT_NTP_FAILED);
        LOGE("NTP server connect failed");
    }
}

void Network::checkCommandFirebase() {
    if (Firebase.RTDB.getInt(&mFbdo, DATA_PATHS.at(0))) {
        REQUEST_FB typeCommand = static_cast<REQUEST_FB>(mFbdo.intData());
        if(typeCommand > REQUEST_FB::SENT_INFORM && typeCommand < REQUEST_FB::REQUEST_END) {
            // setCommandIsIdle();
            switch (typeCommand)
            {
            case REQUEST_FB::GETTING_ALLTIME_DATA: {
                mRML->handleSignal(SignalType::WEB_GET_ALLTIME_DATA_REQUEST);
                break;
            }
            case REQUEST_FB::SETTING_ALLTIME_DATA: {
                // Format:        hour minute second day date month year
                std::vector<int32_t> vecData(7, 0);
                for(int32_t i = 1; i < 8; i++) {
                    if (Firebase.RTDB.getInt(&mFbdo, ALLTIME_PATH.at(i))) {
                        if (mFbdo.dataType() == "int32_t") {
                            vecData.push_back(static_cast<int32_t>(mFbdo.intData()));
                        }
                    }
                    else {
                        LOGE("REASON: %s", mFbdo.errorReason().c_str());
                        return;
                    }
                }
                std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
                // mRML->handleSignal(SignalType::WEB_SET_ALLTIME_DATA_REQUEST, package);
                break;
            }
            case REQUEST_FB::GETTING_LIGHT1_DATA:
            case REQUEST_FB::GETTING_LIGHT2_DATA:
            case REQUEST_FB::GETTING_LIGHT3_DATA:
            case REQUEST_FB::GETTING_LIGHT4_DATA: {
                mRML->handleSignal(mRequestSignalMap[typeCommand].first);
                break;
            }
            case REQUEST_FB::SETTING_LIGHT1_DATA:
            case REQUEST_FB::SETTING_LIGHT2_DATA:
            case REQUEST_FB::SETTING_LIGHT3_DATA:
            case REQUEST_FB::SETTING_LIGHT4_DATA: {
                // Format: swOn hourOn minuteOn secondOn swOff hourOff minuteOff secondOff
                std::vector<int32_t> vecData(8, 0);
                String path;
                for(int32_t i = 1; i < 9; i++)
                {
                    path = LIGHT_PATHS.at(mRequestSignalMap[typeCommand].second) + DATA_PATHS.at(i);
                    if (Firebase.RTDB.getInt(&mFbdo, path.c_str())) {
                        if (mFbdo.dataType() == "int32_t") {
                            vecData.push_back(static_cast<int32_t>(mFbdo.intData()));
                        }
                    }
                    else {
                        LOGE("REASON: %s", mFbdo.errorReason().c_str());
                        return;
                    }
                }
                std::unique_ptr<Package> package = std::make_unique<Package>(vecData);
                // mRML->handleSignal(mRequestSignalMap[typeCommand].first, package);
                break;
            }
            case REQUEST_FB::GETTING_ALL_STATUS: {
                mRML->handleSignal(SignalType::WEB_GET_STATUS_DATA_REQUEST);
                break;
            }
            case REQUEST_FB::SETTING_LIGHT1_STATUS:
            case REQUEST_FB::SETTING_LIGHT2_STATUS:
            case REQUEST_FB::SETTING_LIGHT3_STATUS:
            case REQUEST_FB::SETTING_LIGHT4_STATUS: {
                int32_t data = {0U};
                std::vector<int32_t> vecData;
                LOGI("Setting status of light");
                String path = LIGHT_PATHS.at(5) + DATA_PATHS.at(mRequestSignalMap[typeCommand].second);
                if (Firebase.RTDB.getInt(&mFbdo, path.c_str())) {
                    if (mFbdo.dataType() == "int32_t") {
                        vecData.push_back( static_cast<int32_t>(mFbdo.intData()));
                    }
                }
                else {
                    LOGE("REASON: %s", mFbdo.errorReason().c_str());
                    break;
                }
                std::unique_ptr<Package> package = std::make_unique<Package>(vecData);
                // mRML->handleSignal(mRequestSignalMap[typeCommand].first, package);
                break;
            }
            default:
                break;
            }
        }
    }
    else {
        LOGE("REASON: %s", mFbdo.errorReason().c_str());
        mRML->handleSignal(SignalType::TASKS_CONNECT_RETRY);
        return;
    }
}

void Network::sendAllTimeDatatoWeb(Package *data)
{
    const int32_t SIZE_OF_ALLTIME_DATA = 7U;
    if(data->getSize() != SIZE_OF_ALLTIME_DATA) {
        LOGE("sendAllTimeDatatoWeb(): Length is invalid");
    }
    else {
        const int32_t START_INDEX_ALLTIME_PATH = 1U;
        const int32_t* parseData  = data->getPackage();
        for(int32_t i = 0; i < SIZE_OF_ALLTIME_DATA; i++)
        {
            if (Firebase.RTDB.setInt(&mFbdo, ALLTIME_PATH.at(i + START_INDEX_ALLTIME_PATH), parseData[i])) {
                LOGI("Sent SECOND DATA");
            }
            else {
                LOGE("REASON: %s", mFbdo.errorReason().c_str());
                return;
            }
        }
        sendResponseSetLightDatatoWeb();
    }
}

void Network::sendLightDataToWeb(Package *data, int32_t lightIndex) {
    const int32_t SIZE_OF_LIGHT_DATA = 8U;
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
        const int32_t START_INDEX_LIGHT_PATH = 1U;

        const int32_t* parseData  = data->getPackage();

        for(int32_t i = 0; i < 8; i++)
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
        sendResponseSetLightDatatoWeb();
    }
}

void Network::sendResponseSetLightDatatoWeb() {
    const int32_t INDEX_COMMAND_PATH = 0U;
    if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(INDEX_COMMAND_PATH), static_cast<int32_t>(REQUEST_FB::SENT_INFORM))) {
        LOGI("Sent response");
    }
    else {
        LOGE("FAILED: ", mFbdo.errorReason().c_str());
    }
}

void Network::sendLightStatusToWeb(Package *data) {
    const int32_t SIZE_OF_LIGHT_STATUS = 4U;
    if(data->getSize() != SIZE_OF_LIGHT_STATUS) {
        LOGE("sendLightStatusToWeb(): Length is invalid");
    }
    else {
        String path;
        const int32_t INDEX_STATUS_PATH = 5U;
        const int32_t START_INDEX_STATUS_PATH = 9U;
        const int32_t* parseData  = data->getPackage();
        for(int32_t i = 0; i < SIZE_OF_LIGHT_STATUS; i++)
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
        sendResponseSetLightDatatoWeb();
    }
}

void Network::getTimeDataFromNtp()
{
    LOGI(".");
    // Format: hour minute second day date month year
    std::vector<int32_t> vecData(7, 0);

    mTimeClient->update();
    time_t epochTime = mTimeClient->getEpochTime();

    if (epochTime > 0)
    {
        struct tm *ptm = gmtime(&epochTime);
        vecData[0] = static_cast<int32_t>(ptm->tm_hour);
        vecData[1] = static_cast<int32_t>(ptm->tm_min);
        vecData[2] = static_cast<int32_t>(ptm->tm_sec);
        vecData[3] = static_cast<int32_t>(ptm->tm_wday + 1);
        vecData[4] = static_cast<int32_t>(ptm->tm_mday);
        vecData[5] = static_cast<int32_t>(ptm->tm_mon + 1);   // tm_mon is 0-based
        vecData[6] = static_cast<int32_t>(ptm->tm_year + 1900); // tm_year is years since 1900
    }
    std::unique_ptr<Package> package = std::make_unique<Package>(vecData);
    mRML->handleSignal(SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP, package.get());
}

void Network::setCommandIsIdle() {
    const int32_t INDEX_COMMAND_PATH = 0U;
    // Inform to Firebase server
    if (Firebase.RTDB.setInt(&mFbdo, DATA_PATHS.at(INDEX_COMMAND_PATH), static_cast<int32_t>(REQUEST_FB::IDLE))) {
        LOGI("Sent IDLE COMMAND");
    }
    else {
        LOGE("REASON: %s", mFbdo.errorReason().c_str());
    }
}

void Network::processComboBtnPress() {
    if (mIsConnectedWifi == true)
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        mIsConnectedWifi = false;
        LOGI("WiFi disconnected");
    }

    if (mSerialBT.begin("ESP32_BT"))
    {
        mIsConnectedBT = true;
        mRML->handleSignal(SignalType::LCD_BLUETOOTH_CONNECTED_SUCCESS);
        LOGI("Bluetooth started");
    }
    else
    {
        mIsConnectedBT = false;
        LOGE("Bluetooth start failed");
        mRML->handleSignal(SignalType::LCD_BLUETOOTH_CONNECTED_FAILED);
    }
}

void Network::listenBluetoothData() {
    if (mIsConnectedBT == true)
    {
        if (mSerialBT.available())
        {
            String data = mSerialBT.readStringUntil('\n');
            LOGI("Received from Bluetooth: %s", data.c_str());
        }
    }
}
