#if !defined(alarm_http_handlers_h) && defined(ESP32)
#define alarm_http_handlers_h

#include <internal/net/http_handlers.h>

/////////// HTTP Handlers
void downloadDB();
void deleteDB();

class AlarmHttpHandlers : public HttpHandlers {
    private:
        void defineRoutes();

    public:
        AlarmHttpHandlers(WiFiConnection *wifi, Storage *storage, Settings *settings);

        bool begin();

        // HTTP handlers
        void handleDownloadDB();
        bool handleDeleteDB();

};

extern AlarmHttpHandlers *alarmHttpHandlers;

#endif
