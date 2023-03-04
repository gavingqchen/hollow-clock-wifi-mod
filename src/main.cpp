#include <Arduino.h>
#include <ESPDateTime.h>
#include "Utility.h"
#include "EepromUtility.h"
#include "WifiUtility.h"
#include "FsUtility.h"
#include "WebServer.h"
#include "MotoControl.h"

void setup()
{
    Serial.begin(115200);
    ESP.wdtEnable(30000); // 使能软件看门狗的触发间隔 30s.
    Serial.println("Initialize Timer...");
    TimerInitialize();
    ISR_Timer.setInterval(TIMER_INTERVAL_1S * 10, WatchFeedHandler); // start feed watchdog

    Serial.println("Initialize Eeprom...");
    InitializeEeprom();
    timerClockId = ISR_Timer.setInterval(TIMER_INTERVAL_60S + eepUserSet.clockCompMsPerMin, ISR_TimerHandler_60S);

    Serial.println("Initialize Moto...");
    MotoInitialize();

    Serial.println("Initialize File System...");
    InitializeFileSystem();

    Serial.println("Initialize Wifi..");
    InitializeWifi();
    Serial.println("Start Loop Now...");

}
void loop()
{

    if(runStatus == Startup)
    {
        // Only do during initializing
        WifiConfiguration();
    }
    if(isSyncTriggerred)
    {

    }

    // rotate moto if time change
    CheckTimeAndRotateMoto();
}
