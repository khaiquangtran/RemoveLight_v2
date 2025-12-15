#include "./Network.h"

Network* Network::instance = nullptr;

Network::Network(std::shared_ptr<RemoteLight>rml) : mRML(rml)
{
    mSSID = "";
    mPassword = "";
    mResetProvisioned = true;
    mStatusProvision = false;
    mEnableCheckConnectWIFI = false;
    // WiFi.begin(mSSID, mPassword);
    mConfig.api_key = API_KEY;
    mConfig.database_url = DATABASE_URL;
    instance = this; // save pointer instance
    WiFi.onEvent(SysProvEvent);

    mCommandAllTimerFlag = -1;

    mSignalLightMap = {
        {SignalType::WEB_GET_LIGHT1_DATA_RESPONSE, 1},
        {SignalType::WEB_GET_LIGHT2_DATA_RESPONSE, 2},
        {SignalType::WEB_GET_LIGHT3_DATA_RESPONSE, 3},
        {SignalType::WEB_GET_LIGHT4_DATA_RESPONSE, 4},
    };

    mSettingDataLightMap = {
        {1, SignalType::WEB_SET_LIGHT1_DATA_REQUEST},
        {2, SignalType::WEB_SET_LIGHT2_DATA_REQUEST},
        {3, SignalType::WEB_SET_LIGHT3_DATA_REQUEST},
        {4, SignalType::WEB_SET_LIGHT4_DATA_REQUEST},
    };

    mTimeClient = new NTPClient(ntpUDP, "pool.ntp.org");
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
        break;
    }
    case SignalType::WEB_GET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT4_DATA_RESPONSE: {
        break;
    }
    case SignalType::WEB_SET_ALLTIME_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT4_DATA_RESPONSE:
    case SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE: {
        break;
    }
    case SignalType::WEB_GET_STATUS_DATA_RESPONSE: {
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
    // case SignalType::NETWORK_SEND_SSID_PASSWORD: {
    //     const int32_t* dat = data->getPackage();
    //     const int32_t size = data->getSize();

    //     String str = "";
    //     for (int32_t i =0; i < size; i++) {
    //         str += static_cast<char>(dat[i]);
    //     }

    //     int32_t sep = str.indexOf('%');
    //     if (sep != -1) {
    //         mSSID = str.substring(0, sep);
    //         mPassword = str.substring(sep + 1);

    //         LOGI("SSID: %s", mSSID);
    //         LOGI("Password: %s", mPassword);
    //     }
    //     break;
    // }
    case SignalType::PRESS_BTN_1_2_COMBO_SIGNAL:
    {
        processComboBtnPress();
        break;
    }
    case SignalType::NETWORK_SSID_PASSWORD_STORED: {
        getSSIDAndPasswordFromEEPROM(data);
        break;
    }
    case SignalType::NETWORK_UPLOAD_DATA_TO_FIREBASE: {
        uploadDataToFirebase();
        break;
    }
    default:
        break;
    }
}

void Network::connectWifi()
{
    if (mSSID == "" || mPassword == "")
    {
        LOGE("SSID or Password is empty!");
        mRML->handleSignal(SignalType::TASKS_CONNECT_WIFI_FAILED_SSID_PASSWORD_EMPTY);
    }
    else
    {
        if (mStatusProvision == false)
        {
            WiFi.begin(mSSID, mPassword);
            mEnableCheckConnectWIFI = true;
        }
    }
}

void Network::signUp() {
    // LOGD("Connecting to Firebase...");
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
    mTimeClient->begin();
    mTimeClient->setTimeOffset(GMT);
    if (mTimeClient->update()) {
        mRML->handleSignal(SignalType::TASKS_CONNECT_NTP_SUCCESS);
    }
    else {
        LOGE("NTP server connect failed");
        mRML->handleSignal(SignalType::TASKS_CONNECT_NTP_FAILED);
    }
}

void Network::uploadDataToFirebase() {
    for (std::array<String, 4U>::const_iterator it = HEAD_PATH.begin(); it != HEAD_PATH.end(); ++it)
    {
        std::string flagPath = FLAG_PATH + std::string(it->c_str());
        if (Firebase.RTDB.getInt(&mFbdo, flagPath))
        {
            REQUEST_FB flag = static_cast<REQUEST_FB>(mFbdo.intData());
            if (flag == REQUEST_FB::SET_INFORM)
            {
                std::vector<int32_t> vecData;
                int32_t index = std::distance(HEAD_PATH.begin(), it);
                LOGD("Index: %d", index);
                vecData.push_back(static_cast<int32_t>(index));
                index++;
                for (std::array<String, 8U>::const_iterator dataIt = DATA_PATHS.begin(); dataIt != DATA_PATHS.end(); ++dataIt)
                {
                    std::string path = std::string(it->c_str()) + std::string(dataIt->c_str());
                    LOGD("Path: %s", path.c_str());
                    if (Firebase.RTDB.getInt(&mFbdo, path))
                    {
                        vecData.push_back(static_cast<int32_t>(mFbdo.intData()));
                    }
                    else
                    {
                        LOGE("REASON: %s", mFbdo.errorReason().c_str());
                        return;
                    }
                }

                if (mSettingDataLightMap.find(index) != mSettingDataLightMap.end())
                {
                    std::unique_ptr<Package> package = std::make_unique<Package>(vecData);
                    mRML->handleSignal(mSettingDataLightMap.at(index), package.get());
                    // Set flag received inform
                    if (Firebase.RTDB.setInt(&mFbdo, flagPath.c_str(), static_cast<int32_t>(REQUEST_FB::RECEIVED_INFORM)))
                    {
                        LOGI("Set flag LIGHT_%d to RECEIVED_INFORM", index);
                    }
                    else
                    {
                        LOGE("REASON: %s", mFbdo.errorReason().c_str());
                    }
                }
            }
        }
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

void Network::processComboBtnPress() {
    WiFiProv.beginProvision(
        WIFI_PROV_SCHEME_SOFTAP,
        WIFI_PROV_SCHEME_HANDLER_NONE,
        WIFI_PROV_SECURITY_1,
        POP,
        SERVICE_NAME,
        SERVICE_KEY,
        uuid,
        mResetProvisioned
    );
    WiFiProv.printQR(SERVICE_NAME, POP, "softap");
}

void Network::SysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id)
    {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    {
        uint32_t raw_ip = sys_event->event_info.got_ip.ip_info.ip.addr;
        IPAddress ip(raw_ip);
        LOGD("Connect to WiFi SSID: %s", (const char *)sys_event->event_info.prov_cred_recv.ssid);
        LOGD("Connected IP address : %s", ip.toString().c_str());
        instance->mRML->handleSignal(SignalType::TASKS_CONNECT_WIFI_SUCCESS);
        break;
    }
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    {
        LOGE("Disconnected. Connecting to the AP again... ");
        if (instance->mEnableCheckConnectWIFI) {
            instance->mRML->handleSignal(SignalType::TASKS_CONNECT_WIFI_FAILED);
            instance->mEnableCheckConnectWIFI = false;
        }
        break;
    }
    case ARDUINO_EVENT_PROV_START:
    {
        LOGD("Provisioning started");
        LOGD("Give Credentials of your access point using smartphone app");
        std::string SERVICE_NAME_STR(instance->SERVICE_NAME);
        std::string popStr(instance->POP); // POP should be same as SERVICE_KEY
        std::string dataStr = SERVICE_NAME_STR + "%" + popStr;
        std::unique_ptr<Package> packData = std::make_unique<Package>(dataStr);
        instance->mRML->handleSignal(SignalType::LCD_START_PROVISIONING, packData.get());
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_RECV:
    {
        LOGD("Received Wi-Fi credentials");
        LOGD("SSID : %s", (const char *)sys_event->event_info.prov_cred_recv.ssid);
        LOGD("Password : %s", (const char *)sys_event->event_info.prov_cred_recv.password);
        instance->mSSID = String((const char *)sys_event->event_info.prov_cred_recv.ssid);
        instance->mPassword = String((const char *)sys_event->event_info.prov_cred_recv.password);
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL:
    {
        static int8_t retry = 0;
        if(retry == 0) {
            instance->mRML->handleSignal(SignalType::LCD_PROVISIONING_FAILED);
        }
        retry++;
        if (retry >= MAX_RETRY_PROVISION)
        {
            ESP.restart();
        }
        LOGE("Provisioning failed! Please reset to factory and retry provisioning");
        if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR)
        {
            LOGE("Wi-Fi AP password incorrect");
        }
        else
        {
            LOGE("Wi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
        }
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
    {
        LOGD("Provisioning Successful");
        instance->mRML->handleSignal(SignalType::LCD_PROVISIONING_SUCCESS);
        std::string ssid_password = static_cast<std::string>(instance->mSSID.c_str()) + "%" + static_cast<std::string>(instance->mPassword.c_str());
        std::unique_ptr<Package> packData = std::make_unique<Package>(ssid_password.c_str());
        instance->mRML->handleSignal(SignalType::EEPROM_NETWORK_SEND_SSID_PASSWORD, packData.get());
        break;
    }
    case ARDUINO_EVENT_PROV_END:
    {
        LOGD("Provisioning Ends");
        instance->mStatusProvision = true;
        instance->mRML->handleSignal(SignalType::REMOTE_LIGHT_REMOVE_WIFI_PROVISIONING_MODE);
        break;
    }
    default:
        break;
    }
}

void Network::getSSIDAndPasswordFromEEPROM(const Package *data) {
    if(data == nullptr) {
        LOGE("Data from EEPROM is null.");
        return;
    }
    else {
        const int32_t size = data->getSize();
        const int32_t* value = data->getPackage();
        if(size <= 0) {
            LOGE("Data from EEPROM with length is invalid.");
            return;
        }
        else {
            String str = "";
            for (int32_t i =0; i < size; i++) {
                str += static_cast<char>(value[i]);
            }

            int32_t sep = str.indexOf('%');
            if (sep != -1) {
                mSSID = str.substring(0, sep);
                mPassword = str.substring(sep + 1);

                LOGI("SSID: %s", mSSID.c_str());
                LOGI("Password: %s", mPassword.c_str());
            }
        }
    }
}