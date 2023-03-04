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

    InitializeEeprom();
    timerClockId = ISR_Timer.setInterval(TIMER_INTERVAL_60S + eepUserSet.clockCompMsPerMin, ClockFreeRunModeHandler);
    MotoInitialize();

    InitializeFileSystem();
    InitializeWifi();
}
void loop()
{
    if(runStatus == Startup)
    {
        // Only do during initializing
        WifiConfiguration();
    }


    // rotate moto if time change
}
