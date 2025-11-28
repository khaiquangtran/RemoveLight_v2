#include <Arduino.h>
#include "./SerialPartner.h"

SerialPartner::SerialPartner(std::shared_ptr<RemoteLight> rml) : mRML(rml)
{
    Serial.begin(BAUD_RATE);
    LOGI(" ================== SerialPartner ================== ");
}

SerialPartner::~SerialPartner()
{
}

void SerialPartner::listenning() {
    if (Serial.available()) {

        String receiverData = Serial.readStringUntil('\n');
        receiverData.trim();

        LOGI("%s", receiverData.c_str());

        if (receiverData.equalsIgnoreCase("help") || receiverData.equalsIgnoreCase("h")) {
            LOGI("Available commands:");
            LOGI("help or h - Show this help message");
            LOGI("1 - Send BTN_PRESS_BTN_1_SIGNAL");
            LOGI("2 - Send BTN_PRESS_BTN_2_SIGNAL");
            LOGI("3 - Send PRESS_BTN_1_2_COMBO_SIGNAL");
            LOGI("4 - Clean SSID and Password stored in EEPROM");
            LOGI("=====================================");
        }
        else if (receiverData == "1") {
            mRML->handleSignal(SignalType::BTN_PRESS_BTN_1_SIGNAL);
        }
        else if (receiverData == "2") {
            mRML->handleSignal(SignalType::BTN_PRESS_BTN_2_SIGNAL);
        }
        else if (receiverData == "3") {
            mRML->handleSignal(SignalType::PRESS_BTN_1_2_COMBO_SIGNAL);
        }
        else if (receiverData == "4") {
            mRML->handleSignal(SignalType::EEPROM_CLEAR_SSID_PASSOWRD_DATA);
        }
    }
}


void SerialPartner::handleSignal(const SignalType& signal, const Package *data)
{
    switch (signal)
    {
    case (1):{
        // currentLogLevel = LOG_ERROR;
        break;
    }
    default:
        break;
    }
}
