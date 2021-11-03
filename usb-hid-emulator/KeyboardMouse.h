#pragma once

#include <Arduino.h>
#include "PluggableUSB.h"

#define EPTYPE_DESCRIPTOR_SIZE      uint8_t


class KeyboardMouse_ : public PluggableUSBModule
{
public:
    KeyboardMouse_(void);
    void sendKeyboardReport(uint8_t modifiers, uint8_t keycode1, uint8_t keycode2, uint8_t keycode3, uint8_t keycode4, uint8_t keycode5, uint8_t keycode6);
    void sendMouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel);

protected:
    int getInterface(uint8_t* interfaceCount);
    int getDescriptor(USBSetup& setup);
    bool setup(USBSetup& setup);
    
    EPTYPE_DESCRIPTOR_SIZE epType[1];
    uint8_t protocol;
    uint8_t idle;
    
    void SendReport(void* data, int length);
};

extern KeyboardMouse_ KeyboardMouse;


