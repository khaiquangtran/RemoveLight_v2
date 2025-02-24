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
    if(Firebase.signUp(&mConfig, &mAuth, "", "") == true)
    {
        Firebase.begin(&mConfig, &mAuth);
        mProcessor->handleSignal(SignalType::CONNECT_FIREBASE_SUCCESSFULL);
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
    if (Firebase.RTDB.getInt(&mFbdo, mCommandAllTimerPath.c_str())) {
        if(mFbdo.intData() == 1)
        {
            Firebase.RTDB.setInt(&mFbdo, mCommandAllTimerPath.c_str(), 3);
            mProcessor->handleSignal(SignalType::REQUEST_ALLTIME_DATA);
        }
    }
    else
    {
        Serial.print("error: ");
        Serial.println(mFbdo.errorReason());
    }
}