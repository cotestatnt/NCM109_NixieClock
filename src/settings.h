#include <EEPROM.h>


#ifndef __SETTINGS_H
#define __SETTINGS_H

#define EE_LED_ON   0
#define EE_LED_LOCK 1
#define EE_RED      2
#define EE_GREEN    3
#define EE_BLUE     4

void saveSettings(){
  Print("Saving settings...");
  LED.setSpeed(RGBFadeSpeed*1000);
  LED.setPower((float)RGBPower/100.0);
  uint16_t eeAddress = 0x00;
  EEPROM.put(eeAddress, ledOn);
  eeAddress+=sizeof(bool);
  EEPROM.put(eeAddress, ledLock);
  eeAddress+=sizeof(bool);
  EEPROM.put(eeAddress, RedLight);
  eeAddress+=sizeof(uint8_t);
  EEPROM.put(eeAddress, GreenLight);
  eeAddress+=sizeof(uint8_t);
  EEPROM.put(eeAddress, BlueLight);
  eeAddress+=sizeof(uint8_t);
  EEPROM.put(eeAddress, h_time);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, h_date);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, alarmOn);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, h_time);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, h_date);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, alarmOn);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, alarmHour);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, alarmMin);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, alarmSec);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, tempType);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, tempOffset);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, RGBFadeSpeed);
  eeAddress+=sizeof(int);
  EEPROM.put(eeAddress, RGBPower);
  Println(" done.");
}

void loadSettings(){
  Print("Loading settings...");
  uint16_t eeAddress = 0x00;
  EEPROM.get(eeAddress, ledOn);
  eeAddress+=sizeof(bool);
  EEPROM.get(eeAddress, ledLock);
  eeAddress+=sizeof(bool);
  EEPROM.get(eeAddress, RedLight);
  eeAddress+=sizeof(uint8_t);
  EEPROM.get(eeAddress, GreenLight);
  eeAddress+=sizeof(uint8_t);
  EEPROM.get(eeAddress, BlueLight);
  eeAddress+=sizeof(uint8_t);
  EEPROM.get(eeAddress, h_time);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, h_date);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, alarmOn);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, h_time);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, h_date);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, alarmOn);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, alarmHour);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, alarmMin);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, alarmSec);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, tempType);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, tempOffset);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, RGBFadeSpeed);
  eeAddress+=sizeof(int);
  EEPROM.get(eeAddress, RGBPower);
  Println(" done.");
}



#endif