// Handle DEBUG_ESP_PORT from Arduino IDE
#ifdef DEBUG_ESP_PORT
#ifndef DEBUG_SERIAL
#undef DEBUG_SERIAL
#endif
#ifndef DEBUG_SERIAL_PORT
#undef DEBUG_SERIAL_PORT
#endif
#define DEBUG_SERIAL            1
#define DEBUG_SERIAL_PORT       DEBUG_ESP_PORT
#endif

#ifdef DEBUG_SERIAL
#ifndef DEBUG_SERIAL_PORT
#define DEBUG_SERIAL_PORT       Serial
#endif
#endif

#if DEBUG_SERIAL | DEBUG_UDP
    #define DEBUG_MSG(...) debugSend(__VA_ARGS__)
    #define DEBUG_MSG_P(...) debugSend_P(__VA_ARGS__)
#endif

#ifndef DEBUG_MSG
    #define DEBUG_MSG(...)
    #define DEBUG_MSG_P(...)
#endif
