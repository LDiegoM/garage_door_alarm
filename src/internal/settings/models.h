#ifndef settings_models_h
#define settings_models_h

#include <Arduino.h>
#include <vector>
#include <internal/core/application.h>
#include <internal/core/wifi_connection.h>
#include <internal/core/date_time.h>
#include <internal/core/logging.h>
#include <handlers/mqtt.h>

struct settings_t {
    application_t app;
    handlers_mqtt_t mqtt;
    std::vector<wifiAP_t> wifiAPs;
    dateTime_t dateTime;
    logging_t logging;
};

#endif
