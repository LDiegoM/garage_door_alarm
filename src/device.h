#ifndef device_h
#define device_h

class Device {
    public:
        virtual void setup() = 0;
        virtual void loop() = 0;
};

#endif
