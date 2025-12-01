#include "./EEPROMPartner.h"

EEPROMPartner::EEPROMPartner(std::shared_ptr<RemoteLight> rml) : mRML (rml)
{
    mMapOfDataOfIR = {
        {0,  std::make_pair("IR_BTN_1_SIGNAL",      0)},
        {1,  std::make_pair("IR_BTN_2_SIGNAL",      0)},
        {2,  std::make_pair("IR_BTN_3_SIGNAL",      0)},
        {3,  std::make_pair("IR_BTN_4_SIGNAL",      0)},
        {4,  std::make_pair("IR_BTN_UP_SIGNAL",     0)},
        {5,  std::make_pair("IR_BTN_DOWN_SIGNAL",   0)},
        {6,  std::make_pair("IR_BTN_RIGHT_SIGNAL",  0)},
        {7,  std::make_pair("IR_BTN_LEFT_SIGNAL",   0)},
        {8,  std::make_pair("IR_BTN_OK_SIGNAL",     0)},
        {9,  std::make_pair("IR_BTN_MENU_SIGNAL",   0)},
        {10, std::make_pair("IR_BTN_APP_SIGNAL",    0)},
        {11, std::make_pair("IR_BTN_BACK_SIGNAL",   0)},
        {12, std::make_pair("IR_BTN_5_SIGNAL",      0)},
        {13, std::make_pair("IR_BTN_6_SIGNAL",      0)},
        {14, std::make_pair("IR_BTN_7_SIGNAL",      0)}
    };

    mMapOfLightOnOffTime = {
        {0,   std::make_pair("Light_1_SW_ON",        0)},
        {1,   std::make_pair("Light_1_HOUR_ON",      0)},
        {2,   std::make_pair("Light_1_MINUTE_ON",    0)},
        {3,   std::make_pair("Light_1_SECOND_ON",    0)},
        {4,   std::make_pair("Light_1_SW_OFF",       0)},
        {5,   std::make_pair("Light_1_HOUR_OFF",     0)},
        {6,   std::make_pair("Light_1_MINUTE_OFF",   0)},
        {7,   std::make_pair("Light_1_SECOND_OFF",   0)},
        /*--------------------------------------------*/
        {8,   std::make_pair("Light_2_SW_ON",        0)},
        {9,   std::make_pair("Light_2_HOUR_ON",      0)},
        {10,  std::make_pair("Light_2_MINUTE_ON",    0)},
        {11,  std::make_pair("Light_2_SECOND_ON",    0)},
        {12,  std::make_pair("Light_2_SW_OFF",       0)},
        {13,  std::make_pair("Light_2_HOUR_OFF",     0)},
        {14,  std::make_pair("Light_2_MINUTE_OFF",   0)},
        {15,  std::make_pair("Light_2_SECOND_OFF",   0)},
        /*--------------------------------------------*/
        {16,  std::make_pair("Light_3_SW_ON",        0)},
        {17,  std::make_pair("Light_3_HOUR_ON",      0)},
        {18,  std::make_pair("Light_3_MINUTE_ON",    0)},
        {19,  std::make_pair("Light_3_SECOND_ON",    0)},
        {20,  std::make_pair("Light_3_SW_OFF",       0)},
        {21,  std::make_pair("Light_3_HOUR_OFF",     0)},
        {22,  std::make_pair("Light_3_MINUTE_OFF",   0)},
        {23,  std::make_pair("Light_3_SECOND_OFF",   0)},
        /*--------------------------------------------*/
        {24,  std::make_pair("Light_4_SW_ON",        0)},
        {25,  std::make_pair("Light_4_HOUR_ON",      0)},
        {26,  std::make_pair("Light_4_MINUTE_ON",    0)},
        {27,  std::make_pair("Light_4_SECOND_ON",    0)},
        {28,  std::make_pair("Light_4_SW_OFF",       0)},
        {29,  std::make_pair("Light_4_HOUR_OFF",     0)},
        {30,  std::make_pair("Light_4_MINUTE_OFF",   0)},
        {31,  std::make_pair("Light_4_SECOND_OFF",   0)},
    };
    LOGI(" =========== EEPROMPartner =========== ");
}

void EEPROMPartner::init()
{
    mDataPreferences.begin("ir_data", false);
    std::map<int32_t, std::pair<String, int32_t>>::iterator it;
    for(it = mMapOfDataOfIR.begin(); it != mMapOfDataOfIR.end(); it++)
    {
        it->second.second = mDataPreferences.getInt(it->second.first.c_str(), it->second.second);
    }
    mDataPreferences.end();

    for(int8_t i = 0; i < mMapOfDataOfIR.size(); i++)
    {
        LOGD("Button: %s, data: %x",mMapOfDataOfIR.at(i).first.c_str(), mMapOfDataOfIR.at(i).second);
    }

    mDataPreferences.begin("light_time_data", false);
    for(it = mMapOfLightOnOffTime.begin(); it != mMapOfLightOnOffTime.end(); it++)
    {
        it->second.second = mDataPreferences.getInt(it->second.first.c_str(), it->second.second);
    }
    mDataPreferences.end();

    for(int8_t i = 0; i < mMapOfLightOnOffTime.size(); i++)
    {
        LOGD("%s : %x",mMapOfLightOnOffTime.at(i).first.c_str(), mMapOfLightOnOffTime.at(i).second);
    }

    mDataPreferences.begin("ssid_password", false);
    if (!mDataPreferences.isKey("ssid"))
    {
        mDataPreferences.putString("ssid", "");
        mDataPreferences.putString("password", "");
        mSsid = "";
        mPassword = "";
        LOGW("No stored SSID and Password");
    }
    else
    {
        mSsid = mDataPreferences.getString("ssid", "");
        mPassword = mDataPreferences.getString("password", "");
        LOGI("Storing SSID: %s", mSsid.c_str());
        LOGI("Storing Password: %s", mPassword.c_str());
    }
    mDataPreferences.end();
}

void EEPROMPartner::handleSignal(const SignalType& signal, const Package* data)
{
    switch (signal)
    {
    case (SignalType::EEPROM_IR_REMOTE_GET_DATA):
    {
        std::vector<int32_t> vecData(mMapOfDataOfIR.size());
        for(int32_t i = 0; i < mMapOfDataOfIR.size(); i++)
        {
            vecData[i] = mMapOfDataOfIR.at(i).second;
        }
        std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
        mRML->handleSignal(SignalType::IR_ERRPROM_SEND_DATA, packData.get());
        break;
    }
    case (SignalType::REMOTE_LIGHT_IRBUTTON_INSTALL): {
        if(data->getPackage() == nullptr) {
            LOGE("Data is NULL");
        }
        else {
            const int32_t* value = data->getPackage();
            const int32_t size = data->getSize();
            if(size != 2) {
                LOGE("Length is invalid");
            }
            else {
                mDataPreferences.begin("ir_data", false);
                int32_t numberButton = static_cast<int32_t>(value[0]);
                int32_t dataButton =  static_cast<int32_t>(value[1]);
                mMapOfDataOfIR.at(numberButton).second = dataButton;
                mDataPreferences.putInt(mMapOfDataOfIR.at(numberButton).first.c_str(), dataButton);
                mDataPreferences.end();
            }
        }
        break;
    }
    case SignalType::RTC_STORED_DATA_TO_EPPROM:
    {
        storedDataFromRTC(data);
        break;
    }
    case SignalType::RTC_GET_DATA_FROM_EPPROM:
    {
        sendDataToRTC(data);
        break;
    }
    case SignalType::EEPROM_IS_STORED_SSID_PASSWORD:
    {
        if(mSsid == "" || mPassword == "") {
            LOGW("SSID or Password is empty");
        }
        else {
            String ssid_password = mSsid + String('%') + mPassword;
            std::unique_ptr<Package> packData = std::make_unique<Package>(ssid_password.c_str());
            mRML->handleSignal(SignalType::NETWORK_SSID_PASSWORD_STORED, packData.get());
        }
        break;
    }
    case SignalType::EEPROM_SEND_LIGHT_TIME_ON_OFF_DATA_TO_RTC:
    {
        sendLightOnOffDataToRTC();
        break;
    }
    case SignalType::EEPROM_NETWORK_SEND_SSID_PASSWORD:
    {
        storedDataFromNetwork(data);
        break;
    }
    case SignalType::EEPROM_CLEAR_SSID_PASSOWRD_DATA:
    {
        mDataPreferences.begin("ssid_password", false);
        mDataPreferences.putString("ssid", "");
        mDataPreferences.putString("password", "");
        mDataPreferences.end();
        mSsid = "";
        mPassword = "";
        LOGI("Cleared SSID and Password stored in EEPROM");
        break;
    }
    case SignalType::WEB_SET_LIGHT1_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT2_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT3_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT4_DATA_REQUEST:
    {
        LOGW("5555555");
        storeDataFromSeverFirebase(data);
        break;
    }
    default:
        break;
    }
}

void EEPROMPartner::storedDataFromRTC(const Package* data) {
    if(data == nullptr) {
        LOGE("Data from RTC is null.");
        return;
    }
    else {
        const int32_t size = data->getSize();
        const int32_t *value = data->getPackage();
        if(size != 2) {
            LOGE("Data from RTC with length is not 2.");
            return;
        }
        else {
            mDataPreferences.begin("RTC_data", false);
            std::string key = std::to_string(value[1]);
            mDataPreferences.putUChar(key.c_str(), value[0]);
            mDataPreferences.end();
        }
    }
}

void EEPROMPartner::sendDataToRTC(const Package* data) {
    if(data == nullptr) {
        LOGE("Data from RTC is null.");
        return;
    }
    else {
        const int32_t size = data->getSize();
        const int32_t* value = data->getPackage();
        if(size != 1) {
            LOGE("Data from RTC with length is not 1.");
            return;
        }
        else {
            const int32_t length = 4;
            std::vector<int32_t> vecData(length);
            mDataPreferences.begin("RTC_data", true);
            for (int32_t i = 0; i < length; ++i) {
                std::string keyStr = std::to_string(value[0] + i);
                vecData[i] = mDataPreferences.getUChar(keyStr.c_str(), 0);
            }
            mDataPreferences.end();
            std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
            // mRML->handleSignal(SignalType::EEPROM_SEND_DATA_TO_RTC, &pack);
        }
    }
}

void EEPROMPartner::sendLightOnOffDataToRTC() {
    std::vector<int32_t> vecData(mMapOfLightOnOffTime.size());
    for(int8_t i = 0; i < mMapOfLightOnOffTime.size(); i++)
    {
        vecData.push_back( static_cast<int32_t>(mMapOfLightOnOffTime.at(i).second));
    }
    std::unique_ptr<Package> packData = std::make_unique<Package>(vecData);
    mRML->handleSignal(SignalType::RTC_GET_LIGHT_ON_OFF_DATA, packData.get());
}

void EEPROMPartner::storedDataFromNetwork(const Package* data)
{
    if(data == nullptr) {
        LOGE("Data from Network is null.");
        return;
    }
    else {
        const int32_t size = data->getSize();
        const int32_t* value = data->getPackage();
        if(size <= 0) {
            LOGE("Length is invalid");
            return;
        }
        String str = "";
        for (int32_t i =0; i < size; i++) {
            str += static_cast<char>(value[i]);
        }

        int32_t sep = str.indexOf('%');
        if (sep != -1) {
            mSsid = str.substring(0, sep);
            mPassword = str.substring(sep + 1);

            LOGI("Storing SSID: %s", mSsid.c_str());
            LOGI("Storing Password: %s", mPassword.c_str());

            mDataPreferences.begin("ssid_password", false);
            mDataPreferences.putString("ssid", mSsid.c_str());
            mDataPreferences.putString("password", mPassword.c_str());
            mDataPreferences.end();
        }
    }
}

void EEPROMPartner::storeDataFromSeverFirebase(const Package* data)
{
    if(data == nullptr) {
        LOGE("Data from Server Firebase is null.");
        return;
    }
    else {
        LOGW("6666666");
        const int32_t size = data->getSize();
        LOGW("77777");
        const int32_t* value = data->getPackage();
        LOGW("888888");
        if(size <= 0) {
            LOGE("Length is invalid");
            return;
        }
        else {
            LOGW("7777777");
            mDataPreferences.begin("light_time_data", false);
            size_t indexLight = static_cast<size_t>(value[0]);
            for(int8_t i = 0; i < (size - 1); i++)
            {
                mMapOfLightOnOffTime.at(i + indexLight).second = value[i + 1];
                mDataPreferences.putInt(mMapOfLightOnOffTime.at(i + indexLight).first.c_str(), mMapOfLightOnOffTime.at(i + indexLight).second);
                LOGD("%s : %x",mMapOfLightOnOffTime.at(i + indexLight).first.c_str(), mMapOfLightOnOffTime.at(i + indexLight).second);
            }
            mDataPreferences.end();
        }
    }
}