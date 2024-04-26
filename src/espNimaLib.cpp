#include "espNimaLib.h"
#include <Wire.h> 
#include <LiquidCrystal.h>

// LCD'S PIN CONNECTION
// uint8_t rsLCD = 23;
// uint8_t enLCD = 22;
// uint8_t rwLCD = 32;
// uint8_t d4LCD = 33;
// uint8_t d5LCD = 25;
// uint8_t d6LCD = 26;
// uint8_t d7LCD = 27;

void espNimaLib::LCD_Begin(LiquidCrystal lcd){
    // LiquidCrystal lcd(rsLCD, rwLCD, enLCD, d4LCD, d5LCD, d6LCD, d7LCD);
    lcd.begin(16,2);
    lcd.clear();
    lcd.createChar(0, duck);
    lcd.clear();
    lcd.createChar(0, clock);
    lcd.clear();
}

void espNimaLib::LCD_Write(LiquidCrystal lcd, String dataIn, bool clearLCD= true, uint8_t columnNum= 1){
    // LiquidCrystal lcd(rsLCD, rwLCD, enLCD, d4LCD, d5LCD, d6LCD, d7LCD);
    lcd.setCursor(0, columnNum);
    if(clearLCD == true){
        lcd.clear();
        lcd.setCursor(0, 0);
    }
    lcd.print(dataIn);
}