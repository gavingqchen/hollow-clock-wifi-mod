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

    ESP.wdtEnable(30000); // 使能软件看门狗的触发间隔 30s.
    TimerInitialize();
    ISR_Timer.setInterval(TIMER_INTERVAL_1S * 10, WatchFeedHandler); // start feed watchdog
    Serial.println("Watch dog ready.");
    InitializeEeprom();
    timerClockId = ISR_Timer.setInterval(TIMER_INTERVAL_60S + eepUserSet.clockCompMsPerMin, ISR_TimerHandler_60S);
    MotoInitialize();
    Serial.println("moto ready.");

    InitializeFileSystem();
    InitializeWifi();
    Serial.println("start loop.");

}
void loop()
{
        Serial.println("start loop.");

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
