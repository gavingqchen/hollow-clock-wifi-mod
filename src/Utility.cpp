#include <ESP8266_ISR_Timer.h> //硬件定时器库 https://github.com/khoih-prog/ESP8266TimerInterrupt
#include "Utility.h"
#include "EepromUtility.h"


// Init ESP8266 timer 1
ESP8266Timer ITimer;
// Init ESP8266_ISR_Timer
ESP8266_ISR_Timer ISR_Timer;

uint32_t RTC_minute = 0;             // RTC数据-分钟
uint32_t RTC_hour = 0;               // RTC数据-小时
uint32_t RTC_seconds = 0;            // RTC数据-秒
uint32_t RTC_reboot_count = 0;       // RTC数据-重启计数
uint32_t RTC_wifi_timeout = 0;       // RTC数据-wifi连接超时计数
uint32_t RTC_ntp_error_count = 0;    // 获取时间超时 0-无 1-超时 2-wifi连接超时
uint32_t RTC_ntpTime_cal_status;     // 获取时间超时 0-无 1-已校准, 2-同步超时 3-wifi连接超时

RunStatus runStatus = Startup;             

bool isSyncTriggerred = false;
uint16_t timerClockId;
uint16_t timerSyncId;
uint32_t clockSyncFailureCount = 0;

static uint32_t currTime;
static uint32_t previousTime;
static int32_t offsetMsPerCalInterval = 0;
static uint16_t previousMin;


void TimerInitialize()
{
    if (ITimer.attachInterruptInterval(TIMER_INTERVAL_100MS, TimerHandler))
    {
        currTime = millis();
        previousTime = currTime;
        Serial.print(F("Starting ITimer OK, millis() = "));
        Serial.println(currTime);
    }
    else
        Serial.println(F("Can't set ITimer. Select another freq. or timer"));
}

void IRAM_ATTR TimerHandler()
{
    ISR_Timer.run();
}

void WatchFeedHandler()
{
    ESP.wdtFeed();
}

void ClockFreeRunModeHandler()
{
    uint32_t minute, hour;
    ESP.rtcUserMemoryRead(RTCaddr_hour, &hour, sizeof(hour));
    ESP.rtcUserMemoryRead(RTCaddr_minute, &minute, sizeof(minute));
    currTime = millis();
    if (++minute == 60)
    {
        minute = 0;
        if (++hour == 24)
        {
            hour = 0;
        }
    }
    ESP.rtcUserMemoryWrite(RTCaddr_hour, &hour, sizeof(hour));
    ESP.rtcUserMemoryWrite(RTCaddr_minute, &minute, sizeof(minute));    
}