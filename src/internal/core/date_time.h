#ifndef date_time_h
#define date_time_h

#include <Arduino.h>
#include <time.h>
//#include <esp_sntp.h>

struct dateTime_t {
    String server1;
    String server2;
    long gmtOffset;
    int daylightOffset;
};

class DateTime {
    private:
        char *m_server1;
        char *m_server2;
        int m_gmtOffset;
        int m_daylightOffset;
        tm m_timeInfo;
        String m_year, m_month, m_day, m_hour, m_minutes, m_seconds;

        void setValues();
        bool ESP8266GetLocalTime(struct tm * info, uint32_t ms);

    public:
        DateTime(dateTime_t settings);

        bool begin();
        bool refresh();
        String toString();
        String year();
        String month();
        String day();
        String hour();
        String minutes();
        String seconds();
};

#endif
