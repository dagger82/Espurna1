/*

UTILS MODULE

Copyright (C) 2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

String getIdentifier() {
    char identifier[20];
    sprintf(identifier, "ESPURNA_%06X", ESP.getChipId());
    return String(identifier);
}

float Celsius2Fahrenheit(float celsius) {
    return 32.0 + celsius * 9 / 5;
}

int __get_adc_mode(void) {
    return getSetting("adcMode", ADC_VCC).toInt();
}

void heartbeat() {

    static unsigned long last_uptime = 0;
    static unsigned char uptime_overflows = 0;

    if (millis() < last_uptime) ++uptime_overflows;
    last_uptime = millis();
    unsigned long uptime_seconds = uptime_overflows * (UPTIME_OVERFLOW / 1000) + (last_uptime / 1000);
    unsigned int free_heap = ESP.getFreeHeap();

    DEBUG_MSG_P(PSTR("[MAIN] Time: %s\n"), (char *) NTP.getTimeDateString().c_str());
    if (!mqttConnected()) {
        DEBUG_MSG_P(PSTR("[MAIN] Uptime: %ld seconds\n"), uptime_seconds);
        DEBUG_MSG_P(PSTR("[MAIN] Free heap: %d bytes\n"), free_heap);
        if (__get_adc_mode() == ADC_VCC) {
            DEBUG_MSG_P(PSTR("[MAIN] Power: %d mV\n"), ESP.getVcc());
        }
    }

    #if (MQTT_REPORT_INTERVAL)
        mqttSend(MQTT_TOPIC_INTERVAL, HEARTBEAT_INTERVAL / 1000);
    #endif
    #if (MQTT_REPORT_APP)
        mqttSend(MQTT_TOPIC_APP, APP_NAME);
    #endif
    #if (MQTT_REPORT_VERSION)
        mqttSend(MQTT_TOPIC_VERSION, APP_VERSION);
    #endif
    #if (MQTT_REPORT_HOSTNAME)
        mqttSend(MQTT_TOPIC_HOSTNAME, getSetting("hostname").c_str());
    #endif
    #if (MQTT_REPORT_IP)
        mqttSend(MQTT_TOPIC_IP, getIP().c_str());
    #endif
    #if (MQTT_REPORT_MAC)
        mqttSend(MQTT_TOPIC_MAC, WiFi.macAddress().c_str());
    #endif
    #if (MQTT_REPORT_UPTIME)
        mqttSend(MQTT_TOPIC_UPTIME, String(uptime_seconds).c_str());
    #endif
    #if (MQTT_REPORT_FREEHEAP)
        mqttSend(MQTT_TOPIC_FREEHEAP, String(free_heap).c_str());
    #endif
    #if (MQTT_REPORT_RELAY)
        relayMQTT();
    #endif
    #if (MQTT_REPORT_VCC)
        if (__get_adc_mode() == ADC_VCC) {
            mqttSend(MQTT_TOPIC_VCC, String(ESP.getVcc()).c_str());
        }
    #endif
    #if (MQTT_REPORT_STATUS)
        mqttSend(MQTT_TOPIC_STATUS, MQTT_STATUS_ONLINE);
    #endif

}
