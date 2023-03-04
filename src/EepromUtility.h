#ifndef _EEPROM_UTILITY_H_
#define _EEPROM_UTILITY_H_
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP_EEPROM.h>

#define eeprom_address0 0    //起始地址

struct EEPROMStruct
{
    uint8_t isInitialized;          // eeprom状态 0-首次使用，1-已更新
    int16_t clockCompMsPerMin;      // manual compensation, only valid under runMode==2
    float outputPower;              // 设置发射功率
    uint8_t clockCalInvervalHour;   // 时钟模式校准间隔 小时
};

extern struct EEPROMStruct eepUserSet;
void InitializeEeprom();




#endif
