/*

ESPurna
WEBSERVER MODULE

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

*/

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Hash.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

typedef struct {
    IPAddress ip;
    unsigned long timestamp = 0;
} ws_ticket_t;

ws_ticket_t _ticket[WS_BUFFER_SIZE];

// -----------------------------------------------------------------------------
// WEBSOCKETS
// -----------------------------------------------------------------------------

bool wsSend(char * payload) {
    //DEBUG_MSG("[WEBSOCKET] Broadcasting '%s'\n", payload);
    ws.textAll(payload);
}

bool wsSend(uint32_t client_id, char * payload) {
    //DEBUG_MSG("[WEBSOCKET] Sending '%s' to #%ld\n", payload, client_id);
    ws.text(client_id, payload);
}

void wsMQTTCallback(unsigned int type, const char * topic, const char * payload) {

    if (type == MQTT_CONNECT_EVENT) {
        wsSend((char *) "{\"mqttStatus\": true}");
    }

    if (type == MQTT_DISCONNECT_EVENT) {
        wsSend((char *) "{\"mqttStatus\": false}");
    }

}

void _wsParse(uint32_t client_id, uint8_t * payload, size_t length) {

    // Parse JSON input
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((char *) payload);
    if (!root.success()) {
        DEBUG_MSG("[WEBSOCKET] Error parsing data\n");
        ws.text(client_id, "{\"message\": \"Error parsing data!\"}");
        return;
    }

    // Check actions
    if (root.containsKey("action")) {

        String action = root["action"];
        unsigned int relayID = 0;
        if (root.containsKey("relayID")) {
            String value = root["relayID"];
            relayID = value.toInt();
        }

        DEBUG_MSG("[WEBSOCKET] Requested action: %s\n", action.c_str());

        if (action.equals("reset")) ESP.reset();
        if (action.equals("reconnect")) wifiDisconnect();
        if (action.equals("on")) relayStatus(relayID, true);
        if (action.equals("off")) relayStatus(relayID, false);

    };

    // Check config
    if (root.containsKey("config") && root["config"].is<JsonArray&>()) {

        JsonArray& config = root["config"];
        DEBUG_MSG("[WEBSOCKET] Parsing configuration data\n");

        bool dirty = false;
        bool dirtyMQTT = false;
        bool apiEnabled = false;
        bool timeSaving = false;
        #if ENABLE_FAUXMO
            bool fauxmoEnabled = false;
        #endif
        unsigned int network = 0;
        String adminPass;

        for (unsigned int i=0; i<config.size(); i++) {

            String key = config[i]["name"];
            String value = config[i]["value"];

            if (key == "time") continue;

            if (key == "powExpectedPower") {
                #if ENABLE_POW
                    powSetExpectedActivePower(value.toInt());
                #endif
                continue;
            }

            // Check password
            if (key == "adminPass1") {
                adminPass = value;
                continue;
            }
            if (key == "adminPass2") {
                if (!value.equals(adminPass)) {
                    ws.text(client_id, "{\"message\": \"Passwords do not match!\"}");
                    return;
                }
                if (value.length() == 0) continue;
                ws.text(client_id, "{\"action\": \"reload\"}");
                key = String("adminPass");
            }

            // Checkboxes
            if (key == "apiEnabled") {
                apiEnabled = true;
                continue;
            }
            if (key == "timeSaving") {
                timeSaving = true;
                continue;
            }
            #if ENABLE_FAUXMO
                if (key == "fauxmoEnabled") {
                    fauxmoEnabled = true;
                    continue;
                }
            #endif

            if (key == "ssid") {
                key = key + String(network);
            }
            if (key == "pass") {
                key = key + String(network);
                ++network;
            }

            if (value != getSetting(key)) {
                setSetting(key, value);
                dirty = true;
                if (key.startsWith("mqtt")) dirtyMQTT = true;
            }

        }

        // Checkboxes
        if (apiEnabled != (getSetting("apiEnabled").toInt() == 1)) {
            setSetting("apiEnabled", apiEnabled);
            dirty = true;
        }
        if (timeSaving != (getSetting("timeSaving").toInt() == 1)) {
            setSetting("timeSaving", timeSaving);
            dirty = true;
        }
        #if ENABLE_FAUXMO
            if (fauxmoEnabled != (getSetting("fauxmoEnabled").toInt() == 1)) {
                setSetting("fauxmoEnabled", fauxmoEnabled);
                dirty = true;
            }
        #endif

        // Save settings
        if (dirty) {

            saveSettings();
            wifiConfigure();
            otaConfigure();
            #if ENABLE_FAUXMO
                fauxmoConfigure();
            #endif
            buildTopics();
            ntpConfigure();

            #if ENABLE_RF
                rfBuildCodes();
            #endif

            #if ENABLE_EMON
                setCurrentRatio(getSetting("emonRatio").toFloat());
            #endif

            // Check if we should reconfigure MQTT connection
            if (dirtyMQTT) {
                mqttDisconnect();
            }

            ws.text(client_id, "{\"message\": \"Changes saved\"}");

        } else {

            ws.text(client_id, "{\"message\": \"No changes detected\"}");

        }

    }

}

void _wsStart(uint32_t client_id) {

    char chipid[6];
    sprintf(chipid, "%06X", ESP.getChipId());

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["app"] = APP_NAME;
    root["version"] = APP_VERSION;
    root["buildDate"] = __DATE__;
    root["buildTime"] = __TIME__;

    root["manufacturer"] = String(MANUFACTURER);
    root["chipid"] = chipid;
    root["mac"] = WiFi.macAddress();
    root["device"] = String(DEVICE);
    root["hostname"] = getSetting("hostname", HOSTNAME);
    root["network"] = getNetwork();
    root["ip"] = getIP();

    root["mqttStatus"] = mqttConnected();
    root["mqttServer"] = getSetting("mqttServer", MQTT_SERVER);
    root["mqttPort"] = getSetting("mqttPort", String(MQTT_PORT));
    root["mqttUser"] = getSetting("mqttUser");
    root["mqttPassword"] = getSetting("mqttPassword");
    root["mqttTopic"] = getSetting("mqttTopic", MQTT_TOPIC);

    JsonArray& relay = root.createNestedArray("relayStatus");
    for (unsigned char relayID=0; relayID<relayCount(); relayID++) {
        relay.add(relayStatus(relayID));
    }
    root["relayMode"] = getSetting("relayMode", String(RELAY_MODE));
    if (relayCount() > 1) {
        root["multirelayVisible"] = 1;
        root["relaySync"] = getSetting("relaySync", String(RELAY_SYNC));
    }

    root["apiEnabled"] = getSetting("apiEnabled").toInt() == 1;
    root["apiKey"] = getSetting("apiKey");

    root["timeServer1"] = getSetting("timeServer1", NTP_SERVER);
    root["timeServer2"] = getSetting("timeServer2", "");
    root["timeServer3"] = getSetting("timeServer3", "");
    root["timeZone"] = getSetting("timeZone", String(NTP_TIME_ZONE));
    root["timeSaving"] = getSetting("timeSaving", String(NTP_DAY_LIGHT)).toInt() == 1;
    root["time"] = NTP.getTimeDateString();

    #if ENABLE_FAUXMO
        root["fauxmoVisible"] = 1;
        root["fauxmoEnabled"] = getSetting("fauxmoEnabled", String(FAUXMO_ENABLED)).toInt() == 1;
    #endif

    #if ENABLE_DS18B20
        root["dsVisible"] = 1;
        root["dsTmp"] = getDSTemperature();
    #endif

    #if ENABLE_DHT
        root["dhtVisible"] = 1;
        root["dhtTmp"] = getDHTTemperature();
        root["dhtHum"] = getDHTHumidity();
    #endif

    #if ENABLE_RF
        root["rfVisible"] = 1;
        root["rfChannel"] = getSetting("rfChannel", String(RF_CHANNEL));
        root["rfDevice"] = getSetting("rfDevice", String(RF_DEVICE));
    #endif

    #if ENABLE_EMON
        root["emonVisible"] = 1;
        root["emonPower"] = getPower();
        root["emonMains"] = getSetting("emonMains", String(EMON_MAINS_VOLTAGE));
        root["emonRatio"] = getSetting("emonRatio", String(EMON_CURRENT_RATIO));
    #endif

    #if ENABLE_POW
        root["powVisible"] = 1;
        root["powActivePower"] = getActivePower();
    #endif

    JsonArray& wifi = root.createNestedArray("wifi");
    for (byte i=0; i<3; i++) {
        JsonObject& network = wifi.createNestedObject();
        network["ssid"] = getSetting("ssid" + String(i));
        network["pass"] = getSetting("pass" + String(i));
    }

    String output;
    root.printTo(output);
    ws.text(client_id, (char *) output.c_str());

}

bool _wsAuth(AsyncWebSocketClient * client) {

    IPAddress ip = client->remoteIP();
    unsigned long now = millis();
    unsigned short index = 0;

    for (index = 0; index < WS_BUFFER_SIZE; index++) {
        if ((_ticket[index].ip == ip) && (now - _ticket[index].timestamp < WS_TIMEOUT)) break;
    }

    if (index == WS_BUFFER_SIZE) {
        DEBUG_MSG("[WEBSOCKET] Validation check failed\n");
        ws.text(client->id(), "{\"message\": \"Session expired, please reload page...\"}");
        return false;
    }

    return true;

}

void _wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){

    // Authorize
    #ifndef NOWSAUTH
        if (!_wsAuth(client)) return;
    #endif

    if (type == WS_EVT_CONNECT) {
        IPAddress ip = client->remoteIP();
        DEBUG_MSG("[WEBSOCKET] #%u connected, ip: %d.%d.%d.%d, url: %s\n", client->id(), ip[0], ip[1], ip[2], ip[3], server->url());
        _wsStart(client->id());
    } else if(type == WS_EVT_DISCONNECT) {
        DEBUG_MSG("[WEBSOCKET] #%u disconnected\n", client->id());
    } else if(type == WS_EVT_ERROR) {
        DEBUG_MSG("[WEBSOCKET] #%u error(%u): %s\n", client->id(), *((uint16_t*)arg), (char*)data);
    } else if(type == WS_EVT_PONG) {
        DEBUG_MSG("[WEBSOCKET] #%u pong(%u): %s\n", client->id(), len, len ? (char*) data : "");
    } else if(type == WS_EVT_DATA) {
        _wsParse(client->id(), data, len);
    }

}

// -----------------------------------------------------------------------------
// WEBSERVER
// -----------------------------------------------------------------------------

void _logRequest(AsyncWebServerRequest *request) {
    DEBUG_MSG("[WEBSERVER] Request: %s %s\n", request->methodToString(), request->url().c_str());
}

bool _authenticate(AsyncWebServerRequest *request) {
    String password = getSetting("adminPass", ADMIN_PASS);
    char httpPassword[password.length() + 1];
    password.toCharArray(httpPassword, password.length() + 1);
    return request->authenticate(HTTP_USERNAME, httpPassword);
}

void _onAuth(AsyncWebServerRequest *request) {

    _logRequest(request);

    if (!_authenticate(request)) return request->requestAuthentication();

    IPAddress ip = request->client()->remoteIP();
    unsigned long now = millis();
    unsigned short index;
    for (index = 0; index < WS_BUFFER_SIZE; index++) {
        if (_ticket[index].ip == ip) break;
        if (_ticket[index].timestamp == 0) break;
        if (now - _ticket[index].timestamp > WS_TIMEOUT) break;
    }
    if (index == WS_BUFFER_SIZE) {
        request->send(423);
    } else {
        _ticket[index].ip = ip;
        _ticket[index].timestamp = now;
        request->send(204);
    }

}

void _onHome(AsyncWebServerRequest *request) {

    _logRequest(request);

    if (!_authenticate(request)) return request->requestAuthentication();

    String password = getSetting("adminPass", ADMIN_PASS);
    if (password.equals(ADMIN_PASS)) {
        request->send(SPIFFS, "/password.html");
    } else {
        request->send(SPIFFS, "/index.html");
    }

}

bool _apiAuth(AsyncWebServerRequest *request) {

    if (getSetting("apiEnabled").toInt() == 0) {
        DEBUG_MSG("[WEBSERVER] HTTP API is not enabled\n");
        request->send(403);
        return false;
    }

    if (!request->hasParam("apikey", (request->method() == HTTP_PUT))) {
        DEBUG_MSG("[WEBSERVER] Missing apikey parameter\n");
        request->send(403);
        return false;
    }

    AsyncWebParameter* p = request->getParam("apikey", (request->method() == HTTP_PUT));
    if (!p->value().equals(getSetting("apiKey"))) {
        DEBUG_MSG("[WEBSERVER] Wrong apikey parameter\n");
        request->send(403);
        return false;
    }

    return true;

}

void _onRelay(AsyncWebServerRequest *request) {

    _logRequest(request);

    if (!_apiAuth(request)) return;

    bool asJson = false;
    if (request->hasHeader("Accept")) {
        AsyncWebHeader* h = request->getHeader("Accept");
        asJson = h->value().equals("application/json");
    }

    String output;
    if (asJson) {
        output = relayString();
        request->send(200, "application/json", output);
    } else {
        for (unsigned int i=0; i<relayCount(); i++) {
            output += "Relay #" + String(i) + String(": ") + String(relayStatus(i) ? "1" : "0") + "\n";
        }
        request->send(200, "text/plain", output);
    }

};

ArRequestHandlerFunction _onRelayStatusWrapper(unsigned int relayID) {

    return [relayID](AsyncWebServerRequest *request) {

        _logRequest(request);

        if (!_apiAuth(request)) return;

        if (request->method() == HTTP_PUT) {
            if (request->hasParam("status", true)) {
                AsyncWebParameter* p = request->getParam("status", true);
                wsSend((char *) String(relayID).c_str());
                wsSend((char *) p->value().c_str());
                unsigned int value = p->value().toInt();
                if (value == 2) {
                    relayToggle(relayID);
                } else {
                    relayStatus(relayID, value == 1);
                }
            }
        }

        bool asJson = false;
        if (request->hasHeader("Accept")) {
            AsyncWebHeader* h = request->getHeader("Accept");
            asJson = h->value().equals("application/json");
        }

        String output;
        if (asJson) {
            output = String("{\"relayStatus\": ") + String(relayStatus(relayID) ? "1" : "0") + "}";
            request->send(200, "application/json", output);
        } else {
            request->send(200, "text/plain", relayStatus(relayID) ? "1" : "0");
        }

    };

}

void webSetup() {

    // Setup websocket
    ws.onEvent(_wsEvent);
    mqttRegister(wsMQTTCallback);

    // Setup webserver
    server.addHandler(&ws);

    // Serve home (basic authentication protection)
    server.on("/", HTTP_GET, _onHome);
    server.on("/index.html", HTTP_GET, _onHome);
    server.on("/auth", HTTP_GET, _onAuth);

    // API entry points (protected with apikey)
    for (unsigned int relayID=0; relayID<relayCount(); relayID++) {
        char buffer[15];
        sprintf(buffer, "/api/relay/%d", relayID);
        server.on(buffer, HTTP_GET + HTTP_PUT, _onRelayStatusWrapper(relayID));
    }
    server.on("/api/relay", HTTP_GET, _onRelay);

    // Serve static files
    char lastModified[50];
    sprintf(lastModified, "%s %s GMT", __DATE__, __TIME__);
    server.serveStatic("/", SPIFFS, "/").setLastModified(lastModified);

    // 404
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404);
    });

    // Run server
    server.begin();

}
