/*

DEBUG MODULE

Copyright (C) 2016-2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include <stdio.h>
#include <stdarg.h>

#if DEBUG_UDP
#include <WiFiUdp.h>
WiFiUDP udpDebug;
#endif

void debugSend(const char * format, ...) {

    char buffer[DEBUG_MESSAGE_MAX_LENGTH+1];

    va_list args;
    va_start(args, format);
    ets_vsnprintf(buffer, DEBUG_MESSAGE_MAX_LENGTH, format, args);
    va_end(args);

    #ifdef DEBUG_SERIAL
        if (getBoard() != BOARD_ITEAD_SONOFF_DUAL) {
            DEBUG_SERIAL_PORT.printf(buffer);
        }
    #endif

    #if DEBUG_UDP
        udpDebug.beginPacket(DEBUG_UDP_IP, DEBUG_UDP_PORT);
        udpDebug.write(buffer);
        udpDebug.endPacket();
    #endif

}

void debugSend_P(PGM_P format, ...) {

    char buffer[DEBUG_MESSAGE_MAX_LENGTH+1];

    char f[DEBUG_MESSAGE_MAX_LENGTH+1];
    memcpy_P(f, format, DEBUG_MESSAGE_MAX_LENGTH);

    va_list args;
    va_start(args, format);
    ets_vsnprintf(buffer, DEBUG_MESSAGE_MAX_LENGTH, f, args);
    va_end(args);

    #ifdef DEBUG_SERIAL
        if (getBoard() != BOARD_ITEAD_SONOFF_DUAL) {
            DEBUG_SERIAL_PORT.printf(buffer);
        }
    #endif

    #if DEBUG_UDP
        udpDebug.beginPacket(DEBUG_UDP_IP, DEBUG_UDP_PORT);
        udpDebug.write(buffer);
        udpDebug.endPacket();
    #endif

}
