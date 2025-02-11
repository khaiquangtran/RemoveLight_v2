#include "./WifiPartner.h"
#include <ESP8266WiFi.h>

WifiPartner::WifiPartner(Processor *processor) : mProcessor(processor)
{
    mSSID = WIFI_SSID;
    mPassword = WIFI_PASSWORD;
    mStatusWifi = false;

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool WifiPartner::connectWifi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        mStatusWifi = false;
        Serial.println("Connecting failed");
        return false;
    }
    else
    {
        mStatusWifi = true;
        Serial.println("Connecting successful");
        return true;
    }
}