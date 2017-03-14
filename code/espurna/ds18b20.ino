/*

DS18B20 MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#if ENABLE_DS18B20

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire * ds18b20Wire;
DallasTemperature * ds18b20;

// -----------------------------------------------------------------------------
// Cache
// -----------------------------------------------------------------------------

bool _dsEnabled = false;

// -----------------------------------------------------------------------------
// Provider
// -----------------------------------------------------------------------------

std::vector<DallasTemperature *> _ds18b20s;

unsigned int dsCreate(unsigned int pin) {

    OneWire * wire = new OneWire(pin);
    DallasTemperature * ds18b20 = new DallasTemperature(wire);
    ds18b20->begin();

    #if DEBUG_SERIAL | DEBUG_UDP
        DEBUG_MSG_P(PSTR("[DS18B20] GPIO %d, found %d DS18B20 devices\n"), pin, ds18b20->getDeviceCount());
        DEBUG_MSG_P(PSTR("[DS18B20] GPIO %d, DS18B20 parasite power %s\n"), pin, ds18b20->isParasitePowerMode() ? "ON" : "OFF");
        DeviceAddress address;
        for (int i=0; i<ds18b20->getDeviceCount(); i++) {
            ds18b20->getAddress(address, i);
            DEBUG_MSG_P(PSTR("[DS18B20] GPIO %d, DS18B20 #%d address: "), pin, i);
            for (uint8_t i = 0; i < 8; i++) {
                DEBUG_PORT.printf("%02X", address[i]);
            }
            DEBUG_MSG_P(PSTR("\n"));
        }
    #endif

    _ds18b20s.push_back(ds18b20);
    return _ds18b20s.size() - 1;

}

bool dsGetAddress(uint8_t * address, unsigned int index) {
    unsigned int aggregated = 0;
    unsigned connectionId = 0;
    while (connectionId <= _ds18b20s.size()) {
        unsigned int count = _ds18b20s[connectionId]->getDeviceCount();
        if (aggregated + count > index) {
            _ds18b20s[connectionId]->getAddress(address, index - aggregated);
            return true;
        }
        ++connectionId;
    }
    return false;
}

// TODO:
// Recover async conversion with requestTemperatures() and isConversionComplete()

double dsGetTemperature(unsigned int connectionId, unsigned int index) {
    if (0 <= connectionId && connectionId < _ds18b20s.size()) {
        _ds18b20s[connectionId]->requestTemperatures();
        return _ds18b20s[connectionId]->getTempCByIndex(index);
    }
    return DEVICE_DISCONNECTED_C;
}

double dsGetTemperatureByAddress(uint8_t * address) {
    for (int i=0; i<_ds18b20s.size(); i++) {
        _ds18b20s[i]->requestTemperatures();
        double t = _ds18b20s[i]->getTempC(address);
        if (t != DEVICE_DISCONNECTED_C) return t;
    }
    return DEVICE_DISCONNECTED_C;
}


double dsGetTemperature(unsigned int index) {
    unsigned int aggregated = 0;
    unsigned connectionId = 0;
    while (connectionId <= _ds18b20s.size()) {
        unsigned int count = _ds18b20s[connectionId]->getDeviceCount();
        if (aggregated + count > index) {
            return dsGetTemperature(connectionId, index - aggregated);
        }
        ++connectionId;
    }
    return DEVICE_DISCONNECTED_C;
}

double dsGetTemperature() {
    return dsGetTemperature(0, 0);
}

unsigned int dsCount() {
    int count = 0;
    for (int i=0; i<_ds18b20s.size(); i++) {
        count += _ds18b20s[i]->getDeviceCount();
    }
    return count;
}

// -----------------------------------------------------------------------------
// Setup & Loop
// -----------------------------------------------------------------------------

bool dsEnabled() {
    return _dsEnabled;
}

void dsSetup() {

    _dsEnabled = getSetting("dsEnabled", 0).toInt() == 1;
    if (!_dsEnabled) return;

    dsCreate(getSetting("dsGPIO", DS_PIN).toInt());

    apiRegister("/api/temperature", "temperature", [](char * buffer, size_t len) {
        dtostrf(dsGetTemperature(), len-1, 1, buffer);
    });

    DEBUG_MSG_P(PSTR("[DS18B20] DS18B20 enabled on GPIO #%d\n"), getSetting("dsGPIO", DS_PIN).toInt());


}

void dsLoop() {

    if (!_dsEnabled) return;

    // Check if we should read new data
    static unsigned long last_update = 0;
    if ((millis() - last_update > DS_UPDATE_INTERVAL) || (last_update == 0)) {
        last_update = millis();

        unsigned char tmpUnits = getSetting("tmpUnits", TMP_UNITS).toInt();

        // Read sensor data
        double t = dsGetTemperature(0);

        // Check if readings are valid
        if (t == DEVICE_DISCONNECTED_C) {

            DEBUG_MSG_P(PSTR("[DS18B20] Error reading sensor\n"));

        } else {

            if (tmpUnits == TMP_FAHRENHEIT) t = Celsius2Fahrenheit(t);

            char temperature[6];
            dtostrf(t, 5, 1, temperature);
            DEBUG_MSG_P(PSTR("[DS18B20] Temperature: %s%s\n"), temperature, (tmpUnits == TMP_CELSIUS) ? "ºC" : "ºF");

            // Send MQTT messages
            mqttSend(getSetting("dsTmpTopic", DS_TEMPERATURE_TOPIC).c_str(), temperature);

            // Send to Domoticz
            #if ENABLE_DOMOTICZ
                domoticzSend("dczTmpIdx", 0, temperature);
            #endif

            // Update websocket clients
            char buffer[100];
            sprintf_P(buffer, PSTR("{\"dsVisible\": 1, \"dsTmp\": %s, \"tmpUnits\": %d}"), temperature, tmpUnits);
            wsSend(buffer);

        }

    }

}

#endif
