#ifndef common_mqtt_handlers_h
#define common_mqtt_handlers_h

#include <Arduino.h>

class CommonMqttHandlers {
    public:
        virtual bool isConnected() = 0;
};

#endif
