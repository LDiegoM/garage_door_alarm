#ifndef alarm_device_h
#define alarm_device_h

/*
    Alarm device must detect door opened or closed condition from mqtt queue suscription.
    And ide for multiple main functions in platformio:
    https://community.platformio.org/t/here-is-a-simple-way-to-have-many-small-main-apps/26009
*/
class AlarmDevice {
    private:

    public:
        AlarmDevice();

        void setup();
        void loop();
};

#endif
