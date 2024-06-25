#ifndef button_h
#define button_h

#include <internal/low_level/timer.h>

class Button {
    private:
        byte m_pinButton;
        void (*m_handler)();
        Timer* m_antiBounceTimer;
        byte m_buttonValue;
        bool m_buttonAlreadyPressed;

    public:
        Button(byte pinButton, void (*handler)());
        void check();
    
};

#endif