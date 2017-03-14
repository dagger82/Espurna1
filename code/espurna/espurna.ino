/*

ESPurna

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "config/all.h"
#include <EEPROM.h>

// -----------------------------------------------------------------------------
// BOOTING
// -----------------------------------------------------------------------------

void welcome() {

    DEBUG_MSG_P(PSTR("%s %s\n"), (char *) APP_NAME, (char *) APP_VERSION);
    DEBUG_MSG_P(PSTR("%s\n%s\n\n"), (char *) APP_AUTHOR, (char *) APP_WEBSITE);
    DEBUG_MSG_P(PSTR("ChipID: %06X\n"), ESP.getChipId());
    DEBUG_MSG_P(PSTR("CPU frequency: %d MHz\n"), ESP.getCpuFreqMHz());
    DEBUG_MSG_P(PSTR("Last reset reason: %s\n"), (char *) ESP.getResetReason().c_str());
    DEBUG_MSG_P(PSTR("Memory size: %d bytes\n"), ESP.getFlashChipSize());
    DEBUG_MSG_P(PSTR("Free heap: %d bytes\n"), ESP.getFreeHeap());
    DEBUG_MSG_P(PSTR("Firmware size: %d bytes\n"), ESP.getSketchSize());
    DEBUG_MSG_P(PSTR("Free firmware space: %d bytes\n"), ESP.getFreeSketchSpace());

    #if not EMBEDDED_WEB
        FSInfo fs_info;
        if (SPIFFS.info(fs_info)) {
            DEBUG_MSG_P(PSTR("File system total size: %d bytes\n"), fs_info.totalBytes);
            DEBUG_MSG_P(PSTR("            used size : %d bytes\n"), fs_info.usedBytes);
            DEBUG_MSG_P(PSTR("            block size: %d bytes\n"), fs_info.blockSize);
            DEBUG_MSG_P(PSTR("            page size : %d bytes\n"), fs_info.pageSize);
            DEBUG_MSG_P(PSTR("            max files : %d\n"), fs_info.maxOpenFiles);
            DEBUG_MSG_P(PSTR("            max length: %d\n"), fs_info.maxPathLength);
        }
    #endif

    DEBUG_MSG_P(PSTR("\n"));
    DEBUG_MSG_P(PSTR("Device: %s\n"), getBoardFullName().c_str());
    DEBUG_MSG_P(PSTR("\n"));

}

void setup() {

    // Init SPIFFS
    #if not EMBEDDED_WEB
        SPIFFS.begin();
    #endif

    EEPROM.begin(4096);

    settingsSetup();
    if (getSetting("hostname").length() == 0) {
        setSetting("hostname", getIdentifier());
        saveSettings();
    }
    hwSetup();
    welcome();

    DEBUG_MSG_P(PSTR("[SETUP] ---------------------------------------------\n"));

	webSetup();
    if (getSetting("relayProvider", RELAY_PROVIDER_RELAY).toInt() == RELAY_PROVIDER_LIGHT) {
        lightSetup();
    }
    relaySetup();
    buttonSetup();
    ledSetup();

    delay(500);

    wifiSetup();
    mqttSetup();

    #if ENABLE_OTA
        otaSetup();
    #endif
    #if ENABLE_NTP
        ntpSetup();
    #endif
    #if ENABLE_I2C
        i2cSetup();
    #endif
    #if ENABLE_ALEXA
        fauxmoSetup();
    #endif
    #if ENABLE_NOFUSS
        nofussSetup();
    #endif
    #if ENABLE_HLW8012
        hlwSetup();
    #endif
    #if ENABLE_DS18B20
        dsSetup();
    #endif
    #if ENABLE_DHT
        dhtSetup();
    #endif
    #if ENABLE_RF
        rfSetup();
    #endif
    #if ENABLE_EMON
        emonSetup();
    #endif

    // Configure /api entry point
    webLateSetup();

    DEBUG_MSG_P(PSTR("[SETUP] ---------------------------------------------\n"));

}

void loop() {

    hwLoop();
    buttonLoop();
    ledLoop();
    wifiLoop();
    mqttLoop();

    if (getBoard() != BOARD_ITEAD_SONOFF_DUAL) {
        settingsLoop();
    }

    #if ENABLE_OTA
        otaLoop();
    #endif
    #if ENABLE_NTP
        ntpLoop();
    #endif
    #if ENABLE_ALEXA
        fauxmoLoop();
    #endif
    #if ENABLE_NOFUSS
        nofussLoop();
    #endif
    #if ENABLE_HLW8012
        hlwLoop();
    #endif
    #if ENABLE_DS18B20
        dsLoop();
    #endif
    #if ENABLE_DHT
        dhtLoop();
    #endif
    #if ENABLE_RF
        rfLoop();
    #endif
    #if ENABLE_EMON
        emonLoop();
    #endif

}
