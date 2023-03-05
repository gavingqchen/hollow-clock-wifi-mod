#ifndef _UTILITY_H_
#define _UTILITY_H_
#ifndef DEBUG
#define DEBUG
#endif

#include <NTPClient.h>
#include <WiFiUdp.h>




// Select a Timer Clock
#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               false           // for medium time and medium accurate timer
#define USING_TIM_DIV256              true            // for longest timer but least accurate. Default
// Can be included as many times as necessary, without `Multiple Definitions` Linker Error
#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.hpp"

#define TIMER_INTERVAL_60S              60000L
#define TIMER_INTERVAL_1H               3600000L
#define TIMER_INTERVAL_1S               1000L
#define TIMER_INTERVAL_100MS            100L
#define TIMER_CALI_INTERVAL_MIN         60L

#define RTCaddr_hour                  0                      // RTC数据-地址 小时
#define RTCaddr_minute                RTCaddr_hour+1           // RTC数据-地址 分钟
#define RTCaddr_seconds               RTCaddr_minute+1         // RTC数据-地址 秒
#define RTCaddr_reboot_count          RTCaddr_seconds+1        // RTC数据-地址 重启计数
#define RTCaddr_wifi_timeout          RTCaddr_reboot_count+1     // RTC数据-地址 wifi连接超时计数
#define RTCaddr_ntp_error_count       RTCaddr_wifi_timeout+1      // RTC数据-地址 ntp 同步出错计数
#define RTCaddr_ntpTime_cal_status    RTCaddr_ntp_error_count+1           // RTC数据-地址 NTP错误类型

enum RunStatus{
    Startup = 0,
    FreeRun = 1,
    WiFiConnected = 2,
    ClockSyncMode = 3,
};
extern RunStatus runStatus;

extern NTPClient timeClient;
extern ESP8266_ISR_Timer ISR_Timer;  // declaration of the global variable ISRTimer
extern uint32_t RTC_minute;       // RTC数据-分钟
extern uint32_t RTC_hour;         // RTC数据-小时
extern uint32_t RTC_seconds;      // RTC数据-秒
extern uint32_t RTC_reboot_count;       // RTC数据-重启计数
extern uint32_t RTC_wifi_timeout;      // RTC数据-wifi连接超时计数
extern uint32_t RTC_ntp_error_count;           // RTC数据-ntp同步出错计数
extern uint32_t RTC_ntpTime_cal_status;   // 获取时间超时 0-无 1-已校准, 2-同步超时 3-wifi连接超时
extern bool isSyncTriggerred;
extern uint16_t timerClockId;


void TimerInitialize();
void IRAM_ATTR TimerHandler();
void WatchFeedHandler();
void ISR_TimerHandler_60S();
void TimeCalibrationHandler();
void TimerInitializeAndSync();
void TimerSyncWithNtp();











#endif
