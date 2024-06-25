/*
    ppGame8x8 touch button handling.

    This class implements a handler that deals with anti-bounce and sticky condition.
    
    Diego M. Lopez - 2021 (ldiegom@gmail.com)
*/

#include <internal/low_level/button.h>

Button::Button(byte pinButton, void (*handler)()) {
    m_pinButton = pinButton;
    m_handler = handler;
    m_antiBounceTimer = new Timer(100);

    pinMode(m_pinButton, INPUT_PULLUP);

    m_buttonValue = HIGH;
    m_buttonAlreadyPressed = false;

}

void Button::check() {
    if (m_antiBounceTimer->isRunning()) {
        if (!m_antiBounceTimer->isTime()) {
            return;
        }
        m_antiBounceTimer->stop();
    }
    m_buttonValue = digitalRead(m_pinButton);
    if (m_buttonValue == LOW && !m_buttonAlreadyPressed) {
        m_antiBounceTimer->start();

        // Raise up the sticky button condition.
        m_buttonAlreadyPressed = true;

        // Calls the callback function that handles the button pressed condition.
        m_handler();
    } else if (m_buttonValue == HIGH && m_buttonAlreadyPressed) {
        // Button was released
        m_antiBounceTimer->start();

        // Get down the sticky button condition.
        m_buttonAlreadyPressed = false;
    }
}
