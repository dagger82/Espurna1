/*

LIGHT MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include <Ticker.h>
#include <my9291.h>

Ticker colorTicker;
unsigned char _lightProvider;
bool _lightState = false;
unsigned int _lightColor[3] = {0};
my9291 * _my9291;
unsigned char _redPin;
unsigned char _greenPin;
unsigned char _bluePin;
unsigned char _whitePin;
bool _lightLogic;

// -----------------------------------------------------------------------------
// UTILS
// -----------------------------------------------------------------------------

void color_rgb2array(const char * rgb, unsigned int * array) {

    char * p = (char *) rgb;
    if (p[0] == '#') ++p;

    unsigned long value = strtol(p, NULL, 16);
    array[0] = (value >> 16) & 0xFF;
    array[1] = (value >> 8) & 0xFF;
    array[2] = (value) & 0xFF;

}

void color_array2rgb(unsigned int * array, char * rgb) {
    unsigned long value = array[0];
    value = (value << 8) + array[1];
    value = (value << 8) + array[2];
    sprintf(rgb, "#%06X", value);
}

// -----------------------------------------------------------------------------
// PROVIDER
// -----------------------------------------------------------------------------

void lightColorProvider(unsigned int red, unsigned int green, unsigned int blue) {

    unsigned int white = 0;

    if ((_lightProvider == LIGHT_PROVIDER_MY9192) || (_lightProvider == LIGHT_PROVIDER_RGBW)) {

		// If all set to the same value use white instead
		if ((red == green) && (green == blue)) {
		    white = red;
		    red = green = blue = 0;
		}

    }

    if (_lightProvider == LIGHT_PROVIDER_MY9192) {
        _my9291->setColor((my9291_color_t) { red, green, blue, white });
    }

    if ((_lightProvider == LIGHT_PROVIDER_RGB) || (_lightProvider == LIGHT_PROVIDER_RGBW)) {

        if (_lightLogic) {
            analogWrite(_redPin, red);
            analogWrite(_greenPin, green);
            analogWrite(_bluePin, blue);
            if (_lightProvider == LIGHT_PROVIDER_RGBW) {
	            analogWrite(_whitePin, white);
			}
        } else {
            analogWrite(_redPin, 255 - red);
            analogWrite(_greenPin, 255 - green);
            analogWrite(_bluePin, 255 - blue);
            if (_lightProvider == LIGHT_PROVIDER_RGBW) {
	            analogWrite(_whitePin, 255 - white);
			}
        }

    }

}

// -----------------------------------------------------------------------------
// LIGHT MANAGEMENT
// -----------------------------------------------------------------------------

void lightColor(const char * rgb, bool save, bool forward) {

    color_rgb2array(rgb, _lightColor);
    lightColorProvider(_lightColor[0], _lightColor[1], _lightColor[2]);

    // Delay saving to EEPROM 5 seconds to avoid wearing it out unnecessarily
    if (save) colorTicker.once(LIGHT_SAVE_DELAY, lightColorSave);

    // Report color to MQTT broker
    if (forward) mqttSend(MQTT_TOPIC_COLOR, rgb);

    // Report color to WS clients
    char message[20];
    sprintf(message, "{\"color\": \"%s\"}", rgb);
    wsSend(message);

}

String lightColor() {
    char rgb[8];
    color_array2rgb(_lightColor, rgb);
    return String(rgb);
}

void lightState(bool state) {
    if (state) {
        lightColorProvider(_lightColor[0], _lightColor[1], _lightColor[2]);
    } else {
        lightColorProvider(0, 0, 0);
    }
    _lightState = state;
}

bool lightState() {
    return _lightState;
}

// -----------------------------------------------------------------------------
// PERSISTANCE
// -----------------------------------------------------------------------------

void lightColorSave() {
    setSetting("color", lightColor());
    saveSettings();
}

void lightColorRetrieve() {
    lightColor(getSetting("color", LIGHT_DEFAULT_COLOR).c_str(), false, true);
}

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

void lightMQTTCallback(unsigned int type, const char * topic, const char * payload) {


    if (type == MQTT_CONNECT_EVENT) {
        mqttSubscribe(MQTT_TOPIC_COLOR);
    }

    if (type == MQTT_MESSAGE_EVENT) {

        // Match topic
        String t = mqttSubtopic((char *) topic);
        if (!t.equals(MQTT_TOPIC_COLOR)) return;

        lightColor(payload, true, mqttForward());

    }

}

// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------

void lightSetup() {

    _lightProvider = getSetting("lightProvider", LIGHT_PROVIDER_NONE).toInt();

    if (_lightProvider == LIGHT_PROVIDER_MY9192) {
        unsigned char diPin, dckiPin;
        _my9291 = new my9291(
            diPin = getSetting("myDIGPIO", MY9291_DI_PIN).toInt(),
            dckiPin = getSetting("myDCKIGPIO", MY9291_DCKI_PIN).toInt(),
            MY9291_COMMAND
        );
        DEBUG_MSG_P(PSTR("[LIGHT] MY9291 controller enabled in %d (DI) and %d (DCKI)\n"), diPin, dckiPin);
    }

    if ((_lightProvider == LIGHT_PROVIDER_RGB) || (_lightProvider == LIGHT_PROVIDER_RGBW)) {

        _redPin = getSetting("redGPIO", RGBW_RED_PIN).toInt();
        _greenPin = getSetting("greenGPIO", RGBW_GREEN_PIN).toInt();
        _bluePin = getSetting("blueGPIO", RGBW_BLUE_PIN).toInt();
        _whitePin = getSetting("whiteGPIO", RGBW_WHITE_PIN).toInt();
        _lightLogic = getSetting("lightLogic", 0).toInt();

        pinMode(_redPin, OUTPUT);
        pinMode(_greenPin, OUTPUT);
        pinMode(_bluePin, OUTPUT);
        if (_lightProvider == LIGHT_PROVIDER_RGBW) {
	        pinMode(_whitePin, OUTPUT);
            DEBUG_MSG_P(PSTR("[LIGHT] RGBW controller enabled in GPIO %d, %d, %d and %d\n"), _redPin, _greenPin, _bluePin, _whitePin);
		} else {
            DEBUG_MSG_P(PSTR("[LIGHT] RGB controller enabled in GPIO %d, %d and %d\n"), _redPin, _greenPin, _bluePin);
        }
        lightColorProvider(0, 0, 0);

    }

    lightColorRetrieve();

    mqttRegister(lightMQTTCallback);

}
