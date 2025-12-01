#ifndef EEPROM_PARTNER_H
#define EEPROM_PARTNER_H

#include <Preferences.h>

#include "./Hardware.h"
#include "IRRemotePartner.h"
#include "./../RemoteLight.h"

class EEPROMPartner : public Hardware
{
public:
    EEPROMPartner(std::shared_ptr<RemoteLight> rml);
    EEPROMPartner(const EEPROMPartner &) = delete;
    EEPROMPartner &operator=(const EEPROMPartner &) = delete;
    ~EEPROMPartner() = default;
    void handleSignal(const SignalType& signal, const Package* data = nullptr);
    void init();
    bool isStoredSSIDPassword();

private:
    std::shared_ptr<RemoteLight> mRML;

    Preferences mDataPreferences;
    std::map<int32_t, std::pair<String, int32_t>> mMapOfDataOfIR;
    std::map<int32_t, std::pair<String, int32_t>> mMapOfLightOnOffTime;
    String mSsid;
    String mPassword;

    void storedDataFromRTC(const Package* data);
    void sendDataToRTC(const Package* data);
    void sendLightOnOffDataToRTC();
    void storedDataFromNetwork(const Package* data);
    void storeDataFromSeverFirebase(const Package* data);
};

#endif // EEPROM_PARTNER_H