#include <date_time.h>

//////////////////// Constructor
DateTime::DateTime() {}

//////////////////// Public methods implementation
bool DateTime::refresh() {
#ifdef ESP8266
    if(!ESP8266GetLocalTime(&m_timeInfo, 0))
        return false;
#else
    if(!getLocalTime(&m_timeInfo))
        return false;
#endif

    setValues();
    return true;
}

String DateTime::toString() {
    // Format: YYYY-MM-DD hh:mm:ss
    return year() + "-" + month() + "-" + day() + " " + hour() + ":" + minutes() + ":" + seconds();
}

String DateTime::year() {return m_year;}

String DateTime::month() {return m_month;}

String DateTime::day() {return m_day;}

String DateTime::hour() {return m_hour;}

String DateTime::minutes() {return m_minutes;}

String DateTime::seconds() {return m_seconds;}

//////////////////// Private methods implementation
void DateTime::setValues() {
    char year[5], month[3], day[3], hour[3], minutes[3], seconds[3];
    
    strftime(year, 5, "%Y", &m_timeInfo);
    m_year = String(year);

    strftime(month, 3, "%m", &m_timeInfo);
    m_month = String(month);

    strftime(day, 3, "%d", &m_timeInfo);
    m_day = String(day);

    strftime(hour, 3, "%H", &m_timeInfo);
    m_hour = String(hour);

    strftime(minutes, 3, "%M", &m_timeInfo);
    m_minutes = String(minutes);

    strftime(seconds, 3, "%S", &m_timeInfo);
    m_seconds = String(seconds);
}

bool DateTime::ESP8266GetLocalTime(struct tm * info, uint32_t ms) {
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= ms) {
        time(&now);
        localtime_r(&now, info);
        if(info->tm_year > (2016 - 1900)){
            return true;
        }
        delay(10);
    }
    return false;
}
