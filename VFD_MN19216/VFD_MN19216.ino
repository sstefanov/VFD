
#include <Arduino.h>
#include <SPI.h>
#include <SerialCommands.h>
#include <TimerOne.h>

//#include <mbLog.h>

#include "mbGFX_MN19216.h"

MN19216 canvas; //  uint8_t *getBuffer(void);
char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

void drawSomething()
{
    //  drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),

    // canvas.drawLine(0, 4, 3, 4, 1);
    // LOG << "-------------------\n";
    // byte xx = 1;
    // canvas.drawLine(xx,  8, xx,  11, 1);
    // LOG << "-------------------\n";

    // canvas.drawLine(0,    0,   3,  3, 1);
    // canvas.drawLine(122,  0, 122,  3, 1);
    // canvas.drawLine(0,   31,   3, 28, 1);
    // canvas.drawLine(125, 31, 122, 28, 1);
    // canvas.drawLine(0+32,0+32, 31+32,31+32, 1);
    // canvas.drawLine(0+64,0+64, 31+64,31+64, 1);
    // canvas.drawLine(0+96,0+96, 31+96,31+96, 1);

    canvas.setCursor(7,24);
    canvas.print("@mb");
    canvas.setCursor(42,24);
    canvas.print("VFD:MN19216G");

    canvas.swapBuffers();

    canvas.setCursor(7,24);
    canvas.print("@mb");
    canvas.setCursor(42,24);
    canvas.print("VFD:MN19216G");

    // canvas.swapBuffers();
}

int i = 0;
int dir = 1;
void drawMore()
{
    // // fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    canvas.fillRect(10,0,127,10,0);
    canvas.setCursor(36+i, 1+random(3));
    canvas.setTextSize(1);
    canvas.print("BG AUDIO");
    if(i == 0)
        dir = 1;
    if(i == 32)
        dir = -1;
    i = (i + dir);

    int i12 = (i) / 14 ;
//    canvas.fillCircle(7,10,7,1);
//    canvas.fillCircle(22,10,7,1);
    canvas.fillCircle(7+i12,9+random(3),3,0);
    canvas.fillCircle(22-i12,9+random(3),3,0);
    canvas.drawLine(14,15,15,15,1);

}


/**************************************************************/
void testDisplayRefresh() {
    byte *ptr;
    uint32_t bytes = ((canvas.width()) / 8) * canvas.height();
    uint8_t *drawBuffer;
    drawBuffer = canvas.getBuffer();
    Serial.print("canvas.getBuffer=0x");
    Serial.println((long)drawBuffer, HEX);
    Serial.print("canvas._offset=");
    Serial.println(canvas._offset);
    Serial.print("bytes=");
    Serial.println(bytes);
    drawBuffer = canvas.getBuffer() + canvas._offset;
    Serial.print("drawBuffer=0x");
    Serial.println((long)drawBuffer, HEX);

    byte gate = 0;
    ptr = drawBuffer;
    byte b1;

    Serial.print("ptr=0x");
    Serial.println((long)ptr, HEX);

    Serial.println("gate 1 ptr");
    while (gate < 48) // 1 gate = 4 pixel
    {
//        ptr = drawBuffer; // + gate * 24;
        for (byte i = 0; i < 8; i++) {
            Serial.print(" [");
            Serial.print((long)(ptr), HEX);
            Serial.print("]=0x");
            byte bb = *ptr;
            Serial.print(bb, HEX);
            ptr++;
        }
        gate++;
        Serial.print("gate=");
        Serial.println(gate);
        for (byte i = 0; i < 8; i++) {
            Serial.print(" [");
            Serial.print((long)(ptr), HEX);
            Serial.print("]=0x");
            byte bb = *ptr;
            Serial.print(bb, HEX);
            ptr++;
            //            SPI.transfer(B00001111 & ptr[i]);
        }
        gate++;
        Serial.print("gate=");
        Serial.println(gate);
    }
}

// input offset
// print buffer[offset]
void cmd_r_buf(SerialCommands *sender) {
    char *addr_str = sender->Next();
    if (addr_str == NULL) {
        sender->GetSerial()->println("ERROR NO_ADDRESS");
        return;
    }
    char *length_str = sender->Next();
    uint16_t l;
    if (length_str == NULL)
        l = 1;
    else
        l = (int)strtol(length_str, NULL, 16);

    uint16_t addr = (uint16_t)strtol(addr_str, NULL, 16);
    uint8_t *buf = canvas.getBuffer();
    Serial.print("buf=0x");
    Serial.println((long)buf, HEX);
    uint8_t b;
    for (uint16_t i = 0; i < l; i++) {
        b = buf[addr + i];
        sender->GetSerial()->print("buffer [0x");
        sender->GetSerial()->print((long)(buf + addr + i),HEX);
        sender->GetSerial()->print("] = 0x");
        sender->GetSerial()->println(b,HEX);
    }
}


// input offset, data
void cmd_w_buf(SerialCommands *sender) {
    char *addr_str = sender->Next();
    if (addr_str == NULL) {
        sender->GetSerial()->println("ERROR NO_ADDRESS");
        return;
    }
    uint16_t addr = (uint16_t)strtol(addr_str, NULL, 16);
    char *data_str = sender->Next();
    if (data_str == NULL) {
        sender->GetSerial()->println("ERROR NO_DATA");
        return;
    }
    int data = (int)strtol(data_str, NULL, 16);
    uint16_t l;
    char *length_str = sender->Next();
    if (length_str == NULL)
        l = 1;
    else
        l = (uint16_t)strtol(length_str, NULL, 16);

    uint8_t *buf = canvas.getBuffer();
    Serial.print("buf=0x");
    Serial.println((long)buf, HEX);
    uint8_t b;
    uint8_t b2;
    for (uint16_t i = 0; i < l; i++) {
        b = buf[addr + i];
        buf[addr + i] = data;
        b2 = buf[addr + i];
        sender->GetSerial()->print("buffer [0x");
        sender->GetSerial()->print((long)(buf + addr + i), HEX);
        sender->GetSerial()->print("] changed from 0x");
        sender->GetSerial()->print(b);
        sender->GetSerial()->print(" to 0x");
        sender->GetSerial()->println(b2);
    }
}

// draw pixel x y color
void cmd_p(SerialCommands *sender) {
    char *x_str = sender->Next();
    if (x_str == NULL) {
        sender->GetSerial()->println("ERROR NO_X");
        return;
    }
    uint16_t x = atoi(x_str);

    char *y_str = sender->Next();
    if (y_str == NULL) {
        sender->GetSerial()->println("ERROR NO_Y");
        return;
    }
    uint16_t y = atoi(y_str);

    char *c_str = sender->Next();
    if (c_str == NULL) {
        sender->GetSerial()->println("ERROR NO_COLOR");
        return;
    }
    uint16_t c = atoi(c_str);
    canvas.drawPixel(x, y, c);
}

SerialCommand cmd_r_buf_("R", cmd_r_buf);
SerialCommand cmd_w_buf_("W", cmd_w_buf);
SerialCommand cmd_d_("D", testDisplayRefresh);
SerialCommand cmd_p_("P", cmd_p);
SerialCommand cmd_s_("S", drawSomething);
SerialCommand cmd_m_("M", drawMore);

int16_t x = 0;
int16_t y = 0;
int16_t t = 0;
uint8_t *buff;

void setup() {
    Serial.begin(115200);
    //Wait for console...
    // while (!Serial);
    Serial.println("setup - start");
//    LOG << "LOG setup - start";
    randomSeed(analogRead(0));

    canvas.begin();
    canvas.fillScreen(0);
    serial_commands_.AddCommand(&cmd_r_buf_);
    serial_commands_.AddCommand(&cmd_w_buf_);
    serial_commands_.AddCommand(&cmd_d_);
    serial_commands_.AddCommand(&cmd_p_);
    serial_commands_.AddCommand(&cmd_s_);
    serial_commands_.AddCommand(&cmd_m_);
    // drawSomething();
    // drawMore();
    Timer1.attachInterrupt(canvas.displayRefresh);
    Timer1.initialize(2500);
    buff = canvas.getBuffer() + canvas._offset;
    y = 1;
}

void loop()
{
    serial_commands_.ReadSerial();
    //    canvas.fillScreen(0);
    //    canvas.drawPixel (0,0,255);
    // canvas.drawLine(0,0,63,63,1);
    // canvas.drawLine(64,0,127,63,1);
    /*
    uint8_t *buffer = canvas.getBuffer();
    buffer[0] = 0xfC;
    buffer[1] = 0x0f;
    buffer[2] = 0xc0;

    buffer[3] = 0xfC;
    buffer[4] = 0x0f;
    buffer[5] = 0xc0;
*/
/*
    buff[x] = (uint8_t)y;
    y = y << 1;
    if (y>255) {
        buff[x] = 0;
        x++;
        y = 1;
        if (x >= 192) {
            x = 0;
        }
    }
    Serial.print("x=");
    Serial.print(x);
    Serial.print(", y=0x");
    Serial.println(y, HEX);
    buff[x] = (uint8_t)y;
*/
/*    
    canvas.drawPixel(x, y, 0);
    x++;
    if (x >= 192) {
        x = 0;
        y++;
        if (y >= 16) {
            y=0;
        }
    }

    canvas.drawPixel(x, y, 1);
 //       drawMore();
 //   canvas.swapBuffers();

    //    delay(500);
    //    canvas.swapBuffers();
    //drawSomething();
            delay(100);

    //canvas.drawPixel(0, 0, 0);
    //delay(1000);
*/
//    drawMore();
//    canvas.swapBuffers();
//    delay(1000);
    x += 20 - random(40);
    if (x>191) {
        x = 191;
    }
    if (x < 0) {
        x = 0;
    }
    y += 20 - random(40);
    if (y > 191) {
        y = 191;
    }
    if (y < 0) {
        y = 0;
    }
    byte r = 1; //random(2);
    canvas.fillRect(36, 3, t, 9, 0);
    canvas.fillRect(136, 3, t, 9, 0);
    canvas.setCursor(36 + t, 3 + r);
    canvas.setTextColor(1, 0);
    canvas.setTextSize(1);
    canvas.print("BG AUDIO");
    if (t == 0)
        dir = 1;
    if (t == 42)
        dir = -1;
    t = (t + dir);
    canvas.fillRect(0, 0, x, 3, 1);
    canvas.fillRect(x, 0, 192 - x, 3, 0);

    canvas.fillRect(0, 12, y, 3, 1);
    canvas.fillRect(y, 12, 192 - y, 3, 0);

    delay(2);
}
