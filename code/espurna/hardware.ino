/*

HARDWARE MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

unsigned char _board;

unsigned char getBoard() {
    return _board;
}

String getManufacturer(unsigned char board) {

    if (board >= BOARD_LAST) board = 0;

    // Find size
    size_t len = strlen_P(manufacturers[board]);

    // Copy string
    char buffer[len];
    strcpy_P(buffer, manufacturers[board]);

    return String(buffer);

}

String getManufacturer() {
    return getManufacturer(getBoard());
}

String getBoardName(unsigned char board) {

    if (board >= BOARD_LAST) board = 0;

    // Find size
    size_t len = strlen_P(boardnames[board]);

    // Copy string
    char buffer[len];
    strcpy_P(buffer, boardnames[board]);

    return String(buffer);

}

String getBoardName() {
    return getBoardName(getBoard());
}

String getBoardFullName(unsigned char board) {
    return getManufacturer(board) + String(" ") + getBoardName(board);
}

String getBoardFullName() {
    return getBoardFullName(getBoard());
}

void hwClear(unsigned int index) {
    for (unsigned int i=index; i<MAX_HW_DEVICES; i++) {
        delSetting("ledGPIO", i);
        delSetting("ledLogic", i);
        delSetting("btnGPIO", i);
        delSetting("btnRelay", i);
        delSetting("btnMode", i);
        delSetting("relayGPIO", i);
        delSetting("relayLogic", i);
    }
}

void hwClear() {
    delSetting("relayProvider");
    delSetting("lightProvider");
    delSetting("myDIGPIO");
    delSetting("myDCKIGPIO");
    delSetting("redGPIO");
    delSetting("greenGPIO");
    delSetting("blueGPIO");
    delSetting("whiteGPIO");
    delSetting("lightLogic");
    hwClear(1);
}

void hwLoad(unsigned char board, bool save) {

    // Clean settings
    hwClear();

    // Load settings
    switch (board) {

        case BOARD_CUSTOM:
            break;

        case BOARD_NODEMCU_V3:
            setSetting("ledGPIO", 1, 2);
            setSetting("ledLogic", 1, 1);
            setSetting("btnGPIO", 1, 0);
            setSetting("btnRelay", 1, 1);
            setSetting("relayGPIO", 1, 12);
            setSetting("relayLogic", 1, 0);
            break;

        case BOARD_WEMOS_D1_RELAYSHIELD:
            setSetting("ledGPIO", 1, 2);
            setSetting("ledLogic", 1, 1);
            setSetting("relayGPIO", 1, 5);
            setSetting("relayLogic", 1, 0);
            break;

        case BOARD_ITEAD_SONOFF:
        case BOARD_ITEAD_SONOFF_TH:
        case BOARD_ITEAD_SONOFF_SV:
        case BOARD_ITEAD_SLAMPHER:
        case BOARD_ITEAD_SONOFF_TOUCH:
        case BOARD_ITEAD_S20:
        case BOARD_ITEAD_SONOFF_1CH:
            setSetting("ledGPIO", 1, 13);
            setSetting("ledLogic", 1, 1);
            setSetting("btnGPIO", 1, 0);
            setSetting("btnRelay", 1, 1);
            setSetting("relayGPIO", 1, 12);
            setSetting("relayLogic", 1, 0);
            break;

        case BOARD_ITEAD_SONOFF_POW:
            setSetting("ledGPIO", 1, 15);
            setSetting("ledLogic", 1, 1);
            setSetting("btnGPIO", 1, 0);
            setSetting("btnRelay", 1, 1);
            setSetting("relayGPIO", 1, 12);
            setSetting("relayLogic", 1, 0);
            break;

        case BOARD_ITEAD_SONOFF_DUAL:
            setSetting("ledGPIO", 1, 13);
            setSetting("ledLogic", 1, 1);
            setSetting("btnRelay", 3, 1);
            setSetting("relayProvider", RELAY_PROVIDER_DUAL);
            break;

        case BOARD_ITEAD_SONOFF_4CH:
            setSetting("ledGPIO", 1, 13);
            setSetting("ledLogic", 1, 1);
            setSetting("btnGPIO", 1, 0);
            setSetting("btnGPIO", 2, 9);
            setSetting("btnGPIO", 3, 10);
            setSetting("btnGPIO", 4, 14);
            setSetting("btnRelay", 1, 2);
            setSetting("btnRelay", 2, 3);
            setSetting("btnRelay", 3, 4);
            setSetting("btnRelay", 4, 0);
            setSetting("relayGPIO", 1, 12);
            setSetting("relayGPIO", 2, 5);
            setSetting("relayGPIO", 3, 4);
            setSetting("relayGPIO", 4, 15);
            setSetting("relayLogic", 1, 0);
            setSetting("relayLogic", 2, 0);
            setSetting("relayLogic", 3, 0);
            setSetting("relayLogic", 4, 0);
            break;

        case BOARD_ELECTRODRAGON_ESP_RELAY_BOARD:
            setSetting("ledGPIO", 1, 16);
            setSetting("ledLogic", 1, 0);
            setSetting("btnGPIO", 1, 0);
            setSetting("btnGPIO", 2, 2);
            setSetting("btnRelay", 1, 1);
            setSetting("btnRelay", 2, 2);
            setSetting("relayGPIO", 1, 12);
            setSetting("relayGPIO", 2, 13);
            setSetting("relayLogic", 1, 0);
            setSetting("relayLogic", 2, 0);
            break;

        case BOARD_WORKCHOICE_ECOPLUG:
            setSetting("ledGPIO", 1, 2);
            setSetting("ledLogic", 1, 0);
            setSetting("btnGPIO", 1, 13);
            setSetting("btnRelay", 1, 1);
            setSetting("relayGPIO", 1, 15);
            setSetting("relayLogic", 1, 0);
            break;

        case BOARD_JANGOE_WIFI_RELAY_NC:
        case BOARD_JANGOE_WIFI_RELAY_NO:
            setSetting("btnGPIO", 1, 12);
            setSetting("btnGPIO", 2, 13);
            setSetting("btnRelay", 1, 1);
            setSetting("btnRelay", 2, 2);
            setSetting("relayGPIO", 1, 2);
            setSetting("relayGPIO", 2, 14);
            setSetting("relayLogic", 1, board == BOARD_JANGOE_WIFI_RELAY_NC ? 1 : 0);
            setSetting("relayLogic", 2, board == BOARD_JANGOE_WIFI_RELAY_NC ? 1 : 0);
            break;

        case BOARD_OPENENERGYMONITOR_MQTT_RELAY:
            setSetting("ledGPIO", 1, 16);
            setSetting("ledLogic", 1, 1);
            setSetting("btnGPIO", 1, 0);
            setSetting("btnRelay", 1, 1);
            setSetting("relayGPIO", 1, 12);
            setSetting("relayLogic", 1, 0);
            break;

        case BOARD_JORGE_GARCIA_WIFI_RELAYS_BOARD_KIT:
            setSetting("relayGPIO", 1, 0);
            setSetting("relayLogic", 1, 1);
            setSetting("relayGPIO", 2, 2);
            setSetting("relayLogic", 2, 1);
            break;

        case BOARD_AITHINKER_AILIGHT:
            setSetting("relayProvider", RELAY_PROVIDER_LIGHT);
            setSetting("lightProvider", LIGHT_PROVIDER_MY9192);
            setSetting("myDIGPIO", 13);
            setSetting("myDCKIGPIO", 15);
            break;

        case BOARD_MAGICHOME_LEDCONTROLLER:
            setSetting("relayProvider", RELAY_PROVIDER_LIGHT);
            setSetting("lightProvider", LIGHT_PROVIDER_RGB);
            setSetting("ledGPIO", 1, 2);
            setSetting("ledLogic", 1, 1);
            setSetting("redGPIO", 14);
            setSetting("greenGPIO", 5);
            setSetting("blueGPIO", 12);
            setSetting("whiteGPIO", 13);
            setSetting("lightLogic", 1);
            break;

        default:
            board = 0;
            break;

    }

    // Set & save board
    if (board > 0) {
        setSetting("board", _board = board);
        if (save) saveSettings();
    }

}

void hwLoad(unsigned char board) {
    hwLoad(board, true);
}

void hwSetup() {

    // Check board configuration
    _board = getSetting("board", 0).toInt();
    if (_board == 0) hwLoad(DEFAULT_BOARD);

    // Init serial
    if (_board == BOARD_ITEAD_SONOFF_DUAL) {
        Serial.begin(19230);
    } else {
        Serial.begin(SERIAL_BAUDRATE);
    }

}

void hwLoop() {

    // Heartbeat
    static unsigned long last_heartbeat = 0;
    if ((millis() - last_heartbeat > HEARTBEAT_INTERVAL) || (last_heartbeat == 0)) {
        last_heartbeat = millis();
        heartbeat();
    }

}
