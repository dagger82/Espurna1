/*

DEBUG MODULE

Copyright (C) 2017 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include <stdio.h>
#include <stdarg.h>

#define DEBUG_BUFFER_SIZE 100
#define DEBUG_WS_NONE 0xFF

/*
#ifdef DEBUG_PORT
    #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
    #define DEBUG_MSG(...)
#endif
*/

bool _ws_debug_enabled = true;
unsigned long _ws_debug_client = DEBUG_WS_NONE;

// -----------------------------------------------------------------------------

void DEBUG_WS(bool enable) {
    _ws_debug_enabled = enable;
}

void DEBUG_WS(unsigned int client_id, bool enable) {
    if (enable) {
        _ws_debug_client = client_id;
        DEBUG_MSG("[DEBUG] Debug over websocket for client #%ld enabled\n", _ws_debug_client);
    } else if (_ws_debug_client == client_id) {
        DEBUG_MSG("[DEBUG] Debug over websocket for client #%ld disabled\n", _ws_debug_client);
        _ws_debug_client = DEBUG_WS_NONE;
    }
}

void DEBUG_MSG(const char * format, ...) {

    char b1[DEBUG_BUFFER_SIZE+1];

    va_list args;
    va_start(args, format);
    vsnprintf(b1, DEBUG_BUFFER_SIZE, format, args);
    va_end(args);

    #ifdef DEBUG_PORT
        DEBUG_PORT.print(b1);
    #endif

    if (_ws_debug_enabled & (_ws_debug_client != DEBUG_WS_NONE)) {
        b1[strlen(b1)-1] = 0;
        char b2[strlen(b1) + 15];
        sprintf(b2, "{\"debug\":\"%s\"}", b1);
        wsSend(_ws_debug_client, b2);
    }

}
