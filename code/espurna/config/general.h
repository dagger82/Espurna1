//------------------------------------------------------------------------------
// GENERAL
//------------------------------------------------------------------------------

#define SERIAL_BAUDRATE         115200
#define HOSTNAME                "ESPURNA"
#define BUFFER_SIZE             1024
#define HEARTBEAT_INTERVAL      300000
#define UPTIME_OVERFLOW         4294967295

//--------------------------------------------------------------------------------
// DEBUG
//--------------------------------------------------------------------------------

#define DEBUG_MESSAGE_MAX_LENGTH    80

#define DEBUG_SERIAL_PORT       Serial

// To receive the message son the destination computer use "nc -ul 8111"
#define DEBUG_UDP_IP            IPAddress(192, 168, 1, 100)
#define DEBUG_UDP_PORT          8111

//--------------------------------------------------------------------------------
// EEPROM
//--------------------------------------------------------------------------------

#define EEPROM_RELAY_STATUS     0
#define EEPROM_ENERGY_COUNT     1
#define EEPROM_DATA_END         5

//--------------------------------------------------------------------------------
// BUTTON
//--------------------------------------------------------------------------------

#define BUTTON_LNGCLICK_LENGTH      1000
#define BUTTON_LNGLNGCLICK_LENGTH   10000

#define BUTTON_EVENT_NONE           0
#define BUTTON_EVENT_PRESSED        1
#define BUTTON_EVENT_CLICK          2
#define BUTTON_EVENT_DBLCLICK       3
#define BUTTON_EVENT_LNGCLICK       4
#define BUTTON_EVENT_LNGLNGCLICK    5

#define BUTTON_MODE_NONE            0
#define BUTTON_MODE_TOGGLE          1
#define BUTTON_MODE_AP              2
#define BUTTON_MODE_RESET           3
#define BUTTON_MODE_PULSE           4
#define BUTTON_MODE_FACTORY         5

#define BUTTON_DEFAULT_MODE         BUTTON_MODE_TOGGLE

// Needed for ESP8285 boards under Windows using PlatformIO (?)
#ifndef BUTTON_PUSHBUTTON
#define BUTTON_PUSHBUTTON       0
#define BUTTON_SWITCH           1
#define BUTTON_DEFAULT_HIGH     2
#define BUTTON_SET_PULLUP       4
#endif

//--------------------------------------------------------------------------------
// RELAY
//--------------------------------------------------------------------------------

#define RELAY_MODE_OFF          0
#define RELAY_MODE_ON           1
#define RELAY_MODE_SAME         2
#define RELAY_MODE_TOOGLE       3

#define RELAY_SYNC_ANY          0
#define RELAY_SYNC_NONE_OR_ONE  1
#define RELAY_SYNC_ONE          2
#define RELAY_SYNC_SAME         3

#define RELAY_PULSE_NONE        0
#define RELAY_PULSE_OFF         1
#define RELAY_PULSE_ON          2

#define RELAY_PROVIDER_RELAY    0
#define RELAY_PROVIDER_DUAL     1
#define RELAY_PROVIDER_LIGHT    2

// Pulse time in seconds
#define RELAY_PULSE_TIME        1

// 0 means OFF, 1 ON and 2 whatever was before
#define RELAY_MODE         		RELAY_MODE_OFF

// 0 means ANY, 1 zero or one and 2 one and only one
#define RELAY_SYNC         		RELAY_SYNC_ANY

// 0 means no pulses, 1 means normally off, 2 normally on
#define RELAY_PULSE_MODE     	RELAY_PULSE_NONE

//--------------------------------------------------------------------------------
// I18N
//--------------------------------------------------------------------------------

#define TMP_CELSIUS             0
#define TMP_FAHRENHEIT          1
#define TMP_UNITS               TMP_CELSIUS

//--------------------------------------------------------------------------------
// LED
//--------------------------------------------------------------------------------

// All defined LEDs in the board can be managed through MQTT
// except the first one when LED_AUTO is set to 1.
// If LED_AUTO is set to 1 the board will use first defined LED to show wifi status.
#define LED_AUTO                1

// -----------------------------------------------------------------------------
// WIFI & WEB
// -----------------------------------------------------------------------------

#define WIFI_RECONNECT_INTERVAL 120000
#define WIFI_MAX_NETWORKS       5
#define ADMIN_PASS              "fibonacci"
#define FORCE_CHANGE_PASS       1
#define HTTP_USERNAME           "admin"
#define WS_BUFFER_SIZE          5
#define WS_TIMEOUT              1800000
#define WEBSERVER_PORT          80
#define DNS_PORT                53

#define WEB_MODE_NORMAL         0
#define WEB_MODE_PASSWORD       1

#define AP_MODE                 AP_MODE_ALONE

// This option builds the firmware with the web interface embedded.
// You first have to build the data.h file that holds the contents
// of the web interface by running "gulp buildfs_embed"

#ifndef EMBEDDED_WEB
#define EMBEDDED_WEB            1
#endif

// -----------------------------------------------------------------------------
// OTA & NOFUSS
// -----------------------------------------------------------------------------

#define OTA_PORT                8266
#define NOFUSS_SERVER           ""
#define NOFUSS_INTERVAL         3600000

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

#ifndef MQTT_USE_ASYNC
#define MQTT_USE_ASYNC          1
#endif

#define MQTT_SERVER             ""
#define MQTT_PORT               1883
#define MQTT_TOPIC              "/test/switch/{identifier}"
#define MQTT_RETAIN             true
#define MQTT_QOS                0
#define MQTT_KEEPALIVE          30
#define MQTT_RECONNECT_DELAY    10000
#define MQTT_TRY_INTERVAL       30000
#define MQTT_MAX_TRIES          12
#define MQTT_SKIP_RETAINED      1
#define MQTT_SKIP_TIME          1000

#define MQTT_TOPIC_ACTION       "/action"
#define MQTT_TOPIC_RELAY        "/relay"
#define MQTT_TOPIC_LED          "/led"
#define MQTT_TOPIC_COLOR        "/color"
#define MQTT_TOPIC_BUTTON       "/button"
#define MQTT_TOPIC_IP           "/ip"
#define MQTT_TOPIC_VERSION      "/version"
#define MQTT_TOPIC_UPTIME       "/uptime"
#define MQTT_TOPIC_FREEHEAP     "/freeheap"
#define MQTT_TOPIC_VCC          "/vcc"
#define MQTT_TOPIC_STATUS       "/status"
#define MQTT_TOPIC_MAC          "/mac"
#define MQTT_TOPIC_APP          "/app"
#define MQTT_TOPIC_INTERVAL     "/interval"
#define MQTT_TOPIC_HOSTNAME     "/hostname"

// Periodic reports
#define MQTT_REPORT_STATUS      1
#define MQTT_REPORT_IP          1
#define MQTT_REPORT_MAC         1
#define MQTT_REPORT_UPTIME      1
#define MQTT_REPORT_FREEHEAP    1
#define MQTT_REPORT_VCC         1
#define MQTT_REPORT_RELAY       1
#define MQTT_REPORT_HOSTNAME    1
#define MQTT_REPORT_APP         1
#define MQTT_REPORT_VERSION     1
#define MQTT_REPORT_INTERVAL    0

#define MQTT_STATUS_ONLINE      "1"
#define MQTT_STATUS_OFFLINE     "0"

#define MQTT_ACTION_RESET       "reset"

#define MQTT_CONNECT_EVENT      0
#define MQTT_DISCONNECT_EVENT   1
#define MQTT_MESSAGE_EVENT      2

// Custom get and set postfixes
// Use something like "/status" or "/set", with leading slash
#define MQTT_USE_GETTER         ""
#define MQTT_USE_SETTER         ""

// -----------------------------------------------------------------------------
// SETTINGS
// -----------------------------------------------------------------------------

#define SETTINGS_AUTO_SAVE      0

// -----------------------------------------------------------------------------
// I2C
// -----------------------------------------------------------------------------

#define I2C_SDA_PIN             4
#define I2C_SCL_PIN             14
#define I2C_CLOCK_STRETCH_TIME  200
#define I2C_SCL_FREQUENCY       1000

// -----------------------------------------------------------------------------
// LIGHT
// -----------------------------------------------------------------------------

#define LIGHT_PROVIDER_NONE     0
#define LIGHT_PROVIDER_WS2812   1
#define LIGHT_PROVIDER_RGB      2
#define LIGHT_PROVIDER_RGBW     3
#define LIGHT_PROVIDER_MY9192   4

#define LIGHT_DEFAULT_COLOR     "#000080"
#define LIGHT_SAVE_DELAY        5

#define MY9291_DI_PIN           13
#define MY9291_DCKI_PIN         15
#define MY9291_COMMAND          MY9291_COMMAND_DEFAULT

// Shared settings between RGB and RGBW lights
#define RGBW_INVERSE_LOGIC      1
#define RGBW_RED_PIN            14
#define RGBW_GREEN_PIN          5
#define RGBW_BLUE_PIN           12
#define RGBW_WHITE_PIN          13

// -----------------------------------------------------------------------------
// DOMOTICZ
// -----------------------------------------------------------------------------

#define DOMOTICZ_IN_TOPIC       "domoticz/in"
#define DOMOTICZ_OUT_TOPIC      "domoticz/out"

// -----------------------------------------------------------------------------
// NTP
// -----------------------------------------------------------------------------

#define NTP_SERVER              "pool.ntp.org"
#define NTP_TIME_OFFSET         1
#define NTP_DAY_LIGHT           true
#define NTP_UPDATE_INTERVAL     1800

// -----------------------------------------------------------------------------
// FAUXMO
// -----------------------------------------------------------------------------

#define FAUXMO_START_ACTIVE     1
