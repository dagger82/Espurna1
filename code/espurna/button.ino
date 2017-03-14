/*

BUTTON MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

// -----------------------------------------------------------------------------
// BUTTON
// -----------------------------------------------------------------------------

#include <DebounceEvent.h>
#include <vector>

typedef struct {
    DebounceEvent * button;
    unsigned long actions;
    unsigned int relayID;
} button_t;

std::vector<button_t> _buttons;

#ifdef MQTT_TOPIC_BUTTON
void buttonMQTT(unsigned char id, uint8_t event) {
    if (id >= _buttons.size()) return;
    char payload[2];
    sprintf(payload, "%d", event);
    mqttSend(MQTT_TOPIC_BUTTON, id, payload);
}
#endif

unsigned char buttonAction(unsigned char id, unsigned char event) {
    if (id >= _buttons.size()) return BUTTON_MODE_NONE;
    unsigned long actions = _buttons[id].actions;
    if (event == BUTTON_EVENT_PRESSED) return (actions) & 0x0F;
    if (event == BUTTON_EVENT_CLICK) return (actions >> 4) & 0x0F;
    if (event == BUTTON_EVENT_DBLCLICK) return (actions >> 8) & 0x0F;
    if (event == BUTTON_EVENT_LNGCLICK) return (actions >> 12) & 0x0F;
    if (event == BUTTON_EVENT_LNGLNGCLICK) return (actions >> 16) & 0x0F;
    return BUTTON_MODE_NONE;
}

unsigned int buttonActions(unsigned char id) {

    unsigned char pressAction = getSetting("btnPress", id, BUTTON_MODE_NONE).toInt();
    unsigned char clickAction = getSetting("btnClick", id, BUTTON_MODE_TOGGLE).toInt();
    unsigned char dblClickAction = getSetting("btnDblClick", id, (id == 1) ? BUTTON_MODE_AP : BUTTON_MODE_NONE).toInt();
    unsigned char lngClickAction = getSetting("btnLngClick", id, (id == 1) ? BUTTON_MODE_RESET : BUTTON_MODE_NONE).toInt();
    unsigned char lnglngClickAction = getSetting("btnLngLngClick", id, (id == 1) ? BUTTON_MODE_FACTORY : BUTTON_MODE_NONE).toInt();

    unsigned int value;
    value  = pressAction;
    value += clickAction << 4;
    value += dblClickAction << 8;
    value += lngClickAction << 12;
    value += lnglngClickAction << 16;
    return value;

}

uint8_t mapEvent(uint8_t event, uint8_t count, uint16_t length) {
    if (event == EVENT_PRESSED) return BUTTON_EVENT_PRESSED;
    if (event == EVENT_CHANGED) return BUTTON_EVENT_CLICK;
    if (event == EVENT_RELEASED) {
        if (count == 1) {
            if (length > BUTTON_LNGLNGCLICK_LENGTH) return BUTTON_EVENT_LNGLNGCLICK;
            if (length > BUTTON_LNGCLICK_LENGTH) return BUTTON_EVENT_LNGCLICK;
            return BUTTON_EVENT_CLICK;
        }
        if (count == 2) return BUTTON_EVENT_DBLCLICK;
    }
}

void buttonEvent(unsigned int id, unsigned char event) {

    DEBUG_MSG_P(PSTR("[BUTTON] Pressed #%d, event: %d\n"), id, event);
    if (event == 0) return;

    #ifdef MQTT_TOPIC_BUTTON
        buttonMQTT(id, event);
    #endif

    unsigned char action = buttonAction(id, event);

    if (action == BUTTON_MODE_TOGGLE) {
        if (_buttons[id].relayID > 0) {
            relayToggle(_buttons[id].relayID - 1);
        }
    }
    if (action == BUTTON_MODE_AP) createAP();
    if (action == BUTTON_MODE_RESET) ESP.restart();
    if (action == BUTTON_MODE_PULSE) relayPulseToggle();
    if (action == BUTTON_MODE_FACTORY) {
        DEBUG_MSG_P(PSTR("\n\nFACTORY RESET\n\n"));
        settingsFactoryReset();
        ESP.restart();
    }

}

void buttonSetup() {

    if (getBoard() == BOARD_ITEAD_SONOFF_DUAL) {

        unsigned char relayId = getSetting("btnRelay", 3, 0).toInt();
        _buttons.push_back({NULL, buttonActions(1), 1});
        _buttons.push_back({NULL, buttonActions(2), 2});
        _buttons.push_back({NULL, buttonActions(3), relayId});

    } else {

        unsigned char index = 1;
        while (index < MAX_HW_DEVICES) {
            unsigned char pin = getSetting("btnGPIO", index, GPIO_INVALID).toInt();
            if (pin == GPIO_INVALID) break;
            unsigned char relayId = getSetting("btnRelay", index, 0).toInt();
            unsigned char mode = getSetting("btnMode", index, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH).toInt();
            _buttons.push_back({new DebounceEvent(pin, mode), buttonActions(index), relayId});
            ++index;
        }

    }

    DEBUG_MSG_P(PSTR("[BUTTON] Number of buttons: %d\n"), _buttons.size());

}

void buttonLoop() {

    if (getBoard() == BOARD_ITEAD_SONOFF_DUAL) {

        if (Serial.available() >= 4) {

            unsigned char value;
            if (Serial.read() == 0xA0) {
                if (Serial.read() == 0x04) {
                    value = Serial.read();
                    if (Serial.read() == 0xA1) {

                        // RELAYs and BUTTONs are synchonized in the SIL F330
                        // The on-board BUTTON2 should toggle RELAY0 value
                        // Since we are not passing back RELAY2 value
                        // (in the relayStatus method) it will only be present
                        // here if it has actually been pressed
                        if ((value & 4) == 4) {
                            buttonEvent(2, BUTTON_EVENT_CLICK);
                            return;
                        }

                        // Otherwise check if any of the other two BUTTONs
                        // (in the header) has been pressent, but we should
                        // ensure that we only toggle one of them to avoid
                        // the synchronization going mad
                        // This loop is generic for any PSB-04 module
                        for (unsigned int i=0; i<relayCount(); i++) {

                            bool status = (value & (1 << i)) > 0;

                            // Cjeck if the status for that relay has changed
                            if (relayStatus(i) != status) {
                                buttonEvent(i, BUTTON_EVENT_CLICK);
                                break;
                            }

                        }

                    }
                }
            }
        }

    } else {

        for (unsigned int i=0; i < _buttons.size(); i++) {
            if (unsigned char event = _buttons[i].button->loop()) {
                unsigned char count = _buttons[i].button->getEventCount();
                unsigned long length = _buttons[i].button->getEventLength();
                unsigned char mapped = mapEvent(event, count, length);
                buttonEvent(i, mapped);
            }
        }

    }

}
