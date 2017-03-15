/*

RF MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#if ENABLE_RF

#include <RemoteReceiver.h>

// -----------------------------------------------------------------------------
// Cache
// -----------------------------------------------------------------------------

bool _rfEnabled = false;
unsigned long _rfCode = 0;
unsigned long _rfCodeON = 0;
unsigned long _rfCodeOFF = 0;

// -----------------------------------------------------------------------------
// RF
// -----------------------------------------------------------------------------

void rfLoop() {
    if (_rfCode == 0) return;
    DEBUG_MSG_P(PSTR("[RF] Received code: %lu\n"), _rfCode);
    if (_rfCode == _rfCodeON) relayStatus(0, true);
    if (_rfCode == _rfCodeOFF) relayStatus(0, false);
    _rfCode = 0;
}

void rfBuildCodes() {

    unsigned long code = 0;

    // channel
    unsigned int channel = getSetting("rfChannel", RF_CHANNEL).toInt();
    for (byte i = 0; i < 5; i++) {
        code *= 3;
        if (channel & 1) code += 1;
        channel >>= 1;
    }

    // device
    unsigned int device = getSetting("rfDevice", RF_DEVICE).toInt();
    for (byte i = 0; i < 5; i++) {
        code *= 3;
        if (device != i) code += 2;
    }

    // status
    code *= 9;
    _rfCodeOFF = code + 2;
    _rfCodeON = code + 6;

    DEBUG_MSG_P(PSTR("[RF] Code ON : %lu\n"), _rfCodeON);
    DEBUG_MSG_P(PSTR("[RF] Code OFF: %lu\n"), _rfCodeOFF);

}

void rfCallback(unsigned long code, unsigned int period) {
    _rfCode = code;
}

bool rfEnabled() {
    return _rfEnabled;
}

void rfSetup() {

    _rfEnabled = getSetting("rfEnabled", 0).toInt() == 1;
    if (!_rfEnabled) return;

    unsigned char pin = getSetting("rfGPIO", RF_PIN).toInt();

    pinMode(pin, INPUT_PULLUP);
    rfBuildCodes();
    RemoteReceiver::init(pin, 3, rfCallback);
    RemoteReceiver::disable();
    DEBUG_MSG_P(PSTR("[RF] Disabled\n"));

    static WiFiEventHandler e1 = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
        RemoteReceiver::disable();
        DEBUG_MSG_P(PSTR("[RF] Disabled\n"));
    });

    static WiFiEventHandler e2 = WiFi.onSoftAPModeStationDisconnected([](const WiFiEventSoftAPModeStationDisconnected& event) {
        RemoteReceiver::disable();
        DEBUG_MSG_P(PSTR("[RF] Disabled\n"));
        });

    static WiFiEventHandler e3 = WiFi.onStationModeConnected([](const WiFiEventStationModeConnected& event) {
        RemoteReceiver::enable();
        DEBUG_MSG_P(PSTR("[RF] Enabled\n"));
    });

    static WiFiEventHandler e4 = WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected& event) {
        RemoteReceiver::enable();
        DEBUG_MSG_P(PSTR("[RF] Enabled\n"));
    });

    DEBUG_MSG_P(PSTR("[RF] RF module enabled on GPIO #%d\n"), getSetting("rfGPIO", RF_PIN).toInt());

}

#endif
