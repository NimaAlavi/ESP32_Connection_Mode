#ifndef espNimaLib_H
#define espNimaLib_H

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h> 
#include <LiquidCrystal.h>

class espNimaLib{
    private:
        // LCD's Param
        uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
        uint8_t note[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
        uint8_t clock[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};
        uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
        uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
        uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
        uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
        uint8_t retarrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};
    
    public:
        void LCD_Begin(LiquidCrystal);
        void LCD_Write(LiquidCrystal, String, bool, uint8_t);

};

#endif