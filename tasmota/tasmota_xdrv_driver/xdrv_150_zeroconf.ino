#ifdef USE_WEBSERVER

TasmotaWebServer *ZeroconfServer = nullptr;

void ZeroconfInfo(void);
void ZeroconfSwitch(void);
void ZeroconfStartup(void);
String ZeroconfBuildInfoJson(void);

extern void ExecuteCommandPower(uint32_t device, uint32_t state, uint32_t source);
extern void ExecuteCommand(const char *cmnd, uint32_t source);

void ZeroconfBegin(void)
{
    if (ZeroconfServer) {
        return;
    }

    ZeroconfServer = new TasmotaWebServer(8081);

    ZeroconfServer->on(
        "/zeroconf/info",
        HTTP_POST,
        ZeroconfInfo
    );
    
    ZeroconfServer->on(
        "/zeroconf/switch",
        HTTP_POST,
        ZeroconfSwitch
);

    ZeroconfServer->on(
    "/zeroconf/startup",
    HTTP_POST,
    ZeroconfStartup
);

    ZeroconfServer->begin();
}

String ZeroconfBuildInfoJson(void)
{
    bool relay = bitRead(TasmotaGlobal.power, 0);

    const char *startup =
        (Settings->poweronstate == 0) ? "off" : "on";

    String json;

    json.reserve(1024);

    json = "{\"error\":0,\"data\":{";

    // Estado do relé
    json += "\"switch\":\"";
    json += relay ? "on" : "off";
    json += "\"";

    // Inicialização
    json += ",\"startup\":\"";
    json += startup;
    json += "\"";

    // Nome do dispositivo
    json += ",\"device_name\":\"";
    json += SettingsText(SET_DEVICENAME);
    json += "\"";

    // Firmware personalizado
    json += ",\"firmware\":\"Tasmota DIY by Douglas\"";

    // Versão do firmware
    json += ",\"version\":\"";
    json += TasmotaGlobal.version;
    json += "\"";

    // Data da compilação
    json += ",\"build_date\":\"";
    json += GetBuildDateAndTime();
    json += "\"";

    // Hardware
    json += ",\"hardware\":\"";
    json += GetDeviceHardwareRevision();
    json += "\"";

    // Hostname
    json += ",\"hostname\":\"";
    json += TasmotaGlobal.hostname;
    json += "\"";

    // IP
    json += ",\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\"";

    // MAC
    json += ",\"mac\":\"";
    json += WiFiHelper::macAddress();
    json += "\"";

    // RSSI
    json += ",\"rssi\":";
    json += WiFi.RSSI();

    // Qualidade do WiFi
    json += ",\"wifi_quality\":";
    json += WifiGetRssiAsQuality(WiFi.RSSI());

    // Uptime
    json += ",\"uptime\":\"";
    json += GetUptime();
    json += "\"";

    // Motivo do último reboot
    json += ",\"restart_reason\":\"";
    json += GetResetReason();
    json += "\"";

    // Heap livre
    json += ",\"heap_kb\":";
    json += ESP_getFreeHeap1024();

    // Flash
    json += ",\"flash_kb\":";
    json += ESP.getFlashChipSize() / 1024;

    // CPU
    json += ",\"cpu_mhz\":";
    json += ESP.getCpuFreqMHz();

    // SDK
    json += ",\"sdk\":\"";
    json += ESP.getSdkVersion();
    json += "\"";

    json += "}}";

    return json;
}

void ZeroconfInfo(void)
{

    ZeroconfServer->send(
        200,
        "application/json",
        ZeroconfBuildInfoJson()
    );
}


void ZeroconfSwitch(void)
{
    if (!ZeroconfServer->hasArg("plain")) {

        ZeroconfServer->send(
            400,
            "application/json",
            "{\"error\":1}"
        );

        return;
    }

    String body = ZeroconfServer->arg("plain");

    body.replace(" ", "");
    body.replace("\r", "");
    body.replace("\n", "");

    if (body.indexOf("\"switch\":\"on\"") >= 0) {

        ExecuteCommandPower(
            1,
            POWER_ON,
            SRC_WEB
        );

    } else if (body.indexOf("\"switch\":\"off\"") >= 0) {

        ExecuteCommandPower(
            1,
            POWER_OFF,
            SRC_WEB
        );

    } else {

        ZeroconfServer->send(
            400,
            "application/json",
            "{\"error\":4}"
        );

        return;
    }

    ZeroconfServer->send(
        200,
        "application/json",
        "{\"error\":0}"
    );
}



void ZeroconfStartup(void)
{
    if (!ZeroconfServer->hasArg("plain")) {

        ZeroconfServer->send(
            400,
            "application/json",
            "{\"error\":1}"
        );

        return;
    }

    String body = ZeroconfServer->arg("plain");

    body.replace(" ", "");
    body.replace("\r", "");
    body.replace("\n", "");

    if (body.indexOf("\"startup\":\"on\"") >= 0) {

        ExecuteCommand(
            "PowerOnState 1",
            SRC_WEB
        );

    } else if (body.indexOf("\"startup\":\"off\"") >= 0) {

        ExecuteCommand(
            "PowerOnState 0",
            SRC_WEB
        );

    } else {

        ZeroconfServer->send(
            400,
            "application/json",
            "{\"error\":4}"
        );

        return;
    }

    ZeroconfServer->send(
        200,
        "application/json",
        "{\"error\":0}"
    );
}

#endif