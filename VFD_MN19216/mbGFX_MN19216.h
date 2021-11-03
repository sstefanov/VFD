/*
Mario Becker, 2018, License:MIT.

Code is for TeensyLC, others not tested! Critical things used:
- IntervalTimer
*/

#pragma once

#include <Adafruit_GFX.h>

class MN19216 : public Adafruit_GFX {
public:
    MN19216();
    ~MN19216(void);
    void begin();
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void fillScreen(uint16_t color);
    inline uint8_t *getBuffer(void) { return buffer+_offset; }
    void swapBuffers();

// private:
    static void nextGate(byte gate);
    static void displayRefresh();
    uint8_t *buffer;
    uint32_t _offset = 0;

    static MN19216 *_the;
};


