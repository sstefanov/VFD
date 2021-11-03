/// A GFX 1-bit canvas context for graphics

/// double buffer:
///   buffer+offset is the one to draw, the other will be for display

#include <Arduino.h>
#include <SPI.h>
//#include <mbLog.h>
#include "mbGFX_MN19216.h"
//#include <LowLevelQuickDigitalIO.h>
//using namespace LowLevelQuickDigitalIO;
#include <digitalPinFast.h>

// display spi kann bis 200ns/5MHz geht aber auch mit 16 noch ???
#ifdef __STM32F1__
#else
SPISettings settingsA(16000000, LSBFIRST, SPI_MODE0);
#endif

#define TEST
// MOSI = 11; // PB3
// SCK = 13; // PB5
const byte pinLAT = 8; //PB0;
const byte pinBLK1 = 9; //PB1;
const byte pinBLK2 = 10; //PB2;
digitalPinFast f_pinLAT(pinLAT);
digitalPinFast f_pinBLK1(pinBLK1);
digitalPinFast f_pinBLK2(pinBLK2);
// offset table
const uint8_t _boffset[16] PROGMEM = {
    0x10,
    0x01,
    0x20,
    0x02,
    0x40,
    0x04,
    0x80,
    0x08,
    0x20,
    0x02,
    0x10,
    0x01,
    0x80,
    0x08,
    0x40,
    0x04
};

#ifdef TEST
const byte pinTEST = 4; // Test pin, toggle on timer interrupt
const byte pinTEST2 = 5; // Test pin, toggle on timer interrupt
bool tTEST;
digitalPinFast f_pinTEST(pinTEST);
digitalPinFast f_pinTEST2(pinTEST2);
#endif

//const byte pinGBLK = PA2;   // Not used
//const byte pinGLAT = PA3;   // Not used - VCC1
const byte pinGCLK = 6; //PD6; // A4 -> no output might be related to SPI function
const byte pinGSIN = 7; //PD7; // A6 = MISO seems to be used
digitalPinFast f_pinGCLK(pinGCLK);
digitalPinFast f_pinGSIN(pinGCLK);

const byte pinPWM = 3; // OC2B
#ifdef __STM32F1__
// SCLK=PA5 MOSI=PA7
#else
//#define SDCARD_MOSI_PIN 7
//#define SDCARD_SCK_PIN 14
#endif

// Create an IntervalTimer object 
#ifdef __STM32F1__
HardwareTimer timer(2);
#else
//IntervalTimer myTimer;
#endif

byte *ptr;
unsigned long displayTime;


MN19216 *MN19216::_the = nullptr;


MN19216::MN19216() : Adafruit_GFX(192, 16) {
    uint16_t bytes = (192 / 8) * 16;
    if((buffer = (uint8_t *)malloc(bytes*2))) {
        memset(buffer, 0, bytes*2);
    }
    _the = this;
}

MN19216::~MN19216(void) {
    if(buffer) free(buffer);
}

void MN19216::begin()
{
    pinMode(pinBLK1, OUTPUT);
    pinMode(pinBLK2, OUTPUT);
    pinMode(pinLAT, OUTPUT);
    pinMode(pinGCLK , OUTPUT);
    pinMode(pinGSIN , OUTPUT);
#ifdef TEST
    pinMode(pinTEST, OUTPUT);
    pinMode(pinTEST2, OUTPUT);
#endif
    //    pinMode(pinGBLK , OUTPUT);
    //    pinMode(pinGLAT , OUTPUT);

    digitalWrite(pinBLK1, HIGH);
    digitalWrite(pinBLK2, HIGH);
    digitalWrite(pinLAT, LOW);

    //    digitalWrite(pinGBLK, HIGH);
    //    digitalWrite(pinGLAT, LOW);
    digitalWrite(pinGCLK, HIGH);
    digitalWrite(pinGSIN, LOW);

/*    for(byte i = 0; i < 64; i++)
    {
        digitalWrite(pinGCLK, LOW);
        digitalWrite(pinGCLK, HIGH);
    }
*/
//    digitalWrite(pinGLAT, HIGH);
//    digitalWrite(pinGLAT, LOW);

#ifdef __STM32F1__
    SPI.begin(); //Initiallize the SPI 1 port.
    SPI.setBitOrder(LSBFIRST); // Set the SPI-1 bit order (*)  
    SPI.setDataMode(SPI_MODE0); //Set the  SPI-1 data mode (**)  
    SPI.setClockDivider(SPI_CLOCK_DIV8);		// Slow speed (72 / 16 = 4.5 MHz SPI speed)    timer.pause();
    timer.setPeriod(100000); // in microseconds
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
    timer.attachCompare1Interrupt(displayRefresh);
    timer.refresh();
    timer.resume();
#else
    analogWrite(pinPWM, 128);
    
//    SPI.setMOSI(SDCARD_MOSI_PIN);
//    SPI.setSCK(SDCARD_SCK_PIN);
    SPI.begin();

    SPI.beginTransaction(settingsA);
//    myTimer.begin(displayRefresh, 2000);
//    Timer1.attachInterrupt(displayRefresh);

#endif

}

void MN19216::drawPixel(int16_t x, int16_t y, uint16_t color) {
//    LOG << "drawPixel " << x <<"," << y <<"\n";
    byte *ptr = _the->getBuffer() + _the->_offset;

#ifdef DEBUG
    Serial.print(x);
    Serial.print(", y=");
    Serial.println(y);
#endif

    if (buffer) {
        if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
            return;

        register uint8_t xr = x;
        if (xr < 2)
            xr += 190;
        else
            xr -= 2;
#ifdef DEBUG
        Serial.print("1  xr = ");
        Serial.println(xr);
#endif
        register uint16_t bAddr = (xr / 4) * 8;
#ifdef DEBUG
        Serial.print("1  bAddr:");
        Serial.println(bAddr);
#endif
        register uint8_t bOffset = ((17 - y) / 2) - 1;

#ifdef DEBUG
        Serial.print("1  bOffset = ");
        Serial.println(bOffset);
#endif

        bAddr += bOffset;

#ifdef DEBUG
        Serial.print("1  bAddr:");
        Serial.println(bAddr);
#endif
        uint8_t index = ((xr & 0x07) << 1) + (y & 0x01);
        bOffset = pgm_read_byte(&_boffset[index]);
/*
        Serial.println("_boffset:");
        Serial.print("0x");
        for (uint8_t i = 0; i < sizeof(_boffset); i++) {
            Serial.print(pgm_read_byte(&_boffset[i]), HEX);
            Serial.print(", 0x");
        }
        //        LOG <<"  adr:" <<adr <<", bitpos:" <<bitpos <<" bytepos:" <<bytepos <<"\n";
        Serial.println("");
*/
#ifdef DEBUG
        Serial.print("2  xr & 0x07 << 1:");
        Serial.println((xr & 0x07) << 1);
        Serial.print("2  y & 0x01:");
        Serial.println(y);
        Serial.print("2  index:");
        Serial.println(index);
        Serial.print("2  bOffset = 0x");
        Serial.println(bOffset, HEX);
#endif

        ptr += bAddr;
        int z = (unsigned int)ptr;
#ifdef DEBUG
        Serial.print("  ptr=0x");
        Serial.println(z,HEX);
        Serial.print(" was *ptr=0x");
        Serial.print(*ptr,HEX);
#endif
        if (color)
            *ptr |= bOffset;
        else
            *ptr &= ~bOffset;
#ifdef DEBUG
        Serial.print(" become *ptr=0x");
        Serial.println(*ptr,HEX);
#endif        
    }

}

void MN19216::fillScreen(uint16_t color)
{
    if(buffer) {
        uint16_t bytes = (WIDTH / 8) * HEIGHT;
        memset(buffer + _offset, color ? 0xFF : 0x00, bytes);
    }
}

void MN19216::swapBuffers()
{
    noInterrupts();
    uint16_t bytes = (_width / 8) * _height;
    if(_offset)
    {
        _offset = 0;
    }
    else
    {
        _offset = bytes;
    }

    unsigned long time = displayTime;
    interrupts();

    //LOG << "dispTime:" <<time <<"us\n";
    Serial.print("dispTime:");
    Serial.println(time);

    // 1500us, mhm not so bad

}

void MN19216::nextGate(byte gate)
{
    if(gate < 2)
        digitalWrite(pinGSIN, HIGH);
    else
        digitalWrite(pinGSIN, LOW);
    // clk
    digitalWrite(pinGCLK, LOW);
    digitalWrite(pinGCLK, HIGH);
}

// byte gate = 0;
void MN19216::displayRefresh()
{
#ifdef TEST
    //    tTEST = !tTEST;
    //    digitalWrite(pinTEST, tTEST ? HIGH : LOW);
//    digitalWrite(pinTEST, HIGH);
    f_pinTEST.digitalWriteFast(HIGH);
#endif // TEST
        //    unsigned long time = micros();

    uint32_t bytes = ((_the->width()) / 8) * _the->height();
    byte *drawBuffer = _the->getBuffer() + _the->_offset;

/*                                               if (_the->_offset) {
        drawBuffer = _the->getBuffer() - bytes;
    }
    else
    {
        drawBuffer = _the->getBuffer();// + bytes;
    }
*/
    byte gate = 0;
    ptr = drawBuffer;
    byte b1;

    while (gate < 48) // 1 gate = 4 pixel
    {
        //        nextGate(gate);
        if (gate < 2)
            digitalWrite(pinGSIN, HIGH);
        else
            digitalWrite(pinGSIN, LOW);
        // gclk
        digitalWrite(pinGCLK, LOW);
        digitalWrite(pinGCLK, HIGH);
        digitalWrite(pinGSIN, LOW);
        gate++;

        f_pinBLK1.digitalWriteFast(HIGH);
        f_pinBLK2.digitalWriteFast(HIGH);

        f_pinBLK1.digitalWriteFast(LOW);
        for (byte i = 0; i < 8; i++) {
            SPI.transfer(*ptr);
            ptr++;
//                        SPI.transfer(B11110000 & ptr[i]);
        }
        f_pinBLK1.digitalWriteFast(HIGH);
//        digitalWrite(pinBLK1, HIGH); // delay
//        digitalWrite(pinLAT, HIGH);
//        nextGate(gate);
        if (gate < 2)
            digitalWrite(pinGSIN, HIGH);
        else
            digitalWrite(pinGSIN, LOW);
        // gclk
        digitalWrite(pinGCLK, LOW);
        digitalWrite(pinGCLK, HIGH);
        digitalWrite(pinGSIN, LOW);

        gate ++;

        f_pinLAT.digitalWriteFast(HIGH);
        f_pinLAT.digitalWriteFast(LOW);

        f_pinBLK2.digitalWriteFast(LOW);
        for(byte i = 0; i < 8; i++) {
            SPI.transfer(*ptr);
            ptr++;
    //            SPI.transfer(B00001111 & ptr[i]);
        }
        f_pinBLK2.digitalWriteFast(HIGH);
//        digitalWrite(pinBLK2, HIGH);    // delay
//        digitalWrite(pinLAT, HIGH);

        f_pinLAT.digitalWriteFast(HIGH);
        f_pinLAT.digitalWriteFast(LOW);
    }
    
//    displayTime = micros() - time;;
#ifdef TEST
//    tTEST = !tTEST;
    //    digitalWrite(pinTEST, tTEST ? HIGH : LOW);
//    digitalWrite(pinTEST, LOW);
    f_pinTEST.digitalWriteFast(LOW);
#endif // TEST
}
