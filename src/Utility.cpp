#include <ESP8266_ISR_Timer.h> //硬件定时器库 https://github.com/khoih-prog/ESP8266TimerInterrupt
#include "Utility.h"
#include "EepromUtility.h"
#include "MotoControl.h"
#include "WifiUtility.h"

// Init ESP8266 timer 1
ESP8266Timer ITimer;
// Init ESP8266_ISR_Timer
ESP8266_ISR_Timer ISR_Timer;

WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 8 * 3600, 60000); // udp，服务器地址，时间偏移量，更新间隔
NTPClient timeClient(ntpUDP, "ntp.ntsc.ac.cn", 8 * 3600, 60000); // udp，服务器地址，时间偏移量，更新间隔

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

uint32_t currTime;
uint32_t previousTime;
int32_t  offsetMsPerCalInterval = 0;
unsigned long currSysTimeMs = 0;


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

    ESP.rtcUserMemoryWrite(RTCaddr_hour, &RTC_hour, sizeof(RTC_hour));
    ESP.rtcUserMemoryWrite(RTCaddr_minute, &RTC_minute, sizeof(RTC_minute));
    ESP.rtcUserMemoryWrite(RTCaddr_seconds, &RTC_seconds, sizeof(RTC_seconds));
}

void IRAM_ATTR TimerHandler()
{
    ISR_Timer.run();
}

void WatchFeedHandler()
{
    ESP.wdtFeed();
}
void ISR_TimerHandler_60S()
{
    currSysTimeMs = millis();
    ESP.rtcUserMemoryRead(RTCaddr_hour, &RTC_hour, sizeof(RTC_hour));
    ESP.rtcUserMemoryRead(RTCaddr_minute, &RTC_minute, sizeof(RTC_minute));

    if (++RTC_minute == 60)
    {
        RTC_minute = 0;
        if (++RTC_hour == 24)
        {
            RTC_hour = 0;
        }
        ESP.rtcUserMemoryWrite(RTCaddr_hour, &RTC_hour, sizeof(RTC_hour));
    }
    ESP.rtcUserMemoryWrite(RTCaddr_minute, &RTC_minute, sizeof(RTC_minute));
}

void TimeCalOneShotHandler()
{
    uint32_t minute, hour;
    uint32_t seconds = 0;
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
    // Seconds will be aligned after oneshot handler
    ESP.rtcUserMemoryWrite(RTCaddr_seconds, &seconds, sizeof(seconds));

    timerClockId = ISR_Timer.setInterval(TIMER_INTERVAL_60S + offsetMsPerCalInterval / TIMER_CALI_INTERVAL_MIN, ISR_TimerHandler_60S);
    timerSyncId = ISR_Timer.setInterval(TIMER_CALI_INTERVAL_MIN * 60000 + offsetMsPerCalInterval, TimeCalibrationHandler);

#ifdef DEBUG
    Serial.print(("One Shot Handler called, offsetMsPerMin is:  " + String(offsetMsPerCalInterval / TIMER_CALI_INTERVAL_MIN) + "\r\n"));
    Serial.print(("TimeCalOneShotHandler, millis() = "));
    Serial.println(currTime);
    Serial.print(("TimeCalOneShotHandler Time:  " + String(hour) + ":" + String(minute) + "\r\n"));
#endif
}

void TimeCalibrationHandler()
{
    isSyncTriggerred = true;
}


void TimerInitializeAndSync()
{
    RTC_ntp_error_count = 0;
    RTC_ntpTime_cal_status = 0;
    unsigned long clockPrevious = 0, clockAfter = 0;
    uint32_t hourNtp, minuteNtp, secondsNtp;
    uint32_t minute, hour;
    uint16_t syncRetryCount = 0;
    bool syncSuccessFlag = false;

    timeClient.begin();
    do
    {
        clockPrevious = millis();
        syncSuccessFlag = timeClient.update();
        if (!syncSuccessFlag)
            continue;
        hourNtp = timeClient.getHours();
        minuteNtp = timeClient.getMinutes();
        secondsNtp = timeClient.getSeconds();

        // NTP lib seems will return strange value sometimes
        if (hourNtp > 24 || minuteNtp > 60 || secondsNtp > 60)
        {
            syncSuccessFlag = false;
            continue;
        }
        clockAfter = millis();
        if (syncSuccessFlag)
            break;
    } while (++syncRetryCount <= 15);

    if (syncSuccessFlag)
    {
        ISR_Timer.disable(timerClockId);
        ISR_Timer.deleteTimer(timerClockId);

        ESP.rtcUserMemoryRead(RTCaddr_hour, &hour, sizeof(hour));
        ESP.rtcUserMemoryRead(RTCaddr_minute, &minute, sizeof(minute));

// Debug info
#ifdef DEBUG
        Serial.print(("Current SYS Time:  " + String(hour) + ":" + String(minute) + "\r\n"));
        Serial.print(("Starting NTP Initialize, millis() = "));
        Serial.println(clockPrevious);
        Serial.print(("NTP millis() = :  "));
        Serial.println(clockAfter);
        Serial.print(("NTP TIme:  " + String(hourNtp) + ":" + String(minuteNtp) + ":" + String(secondsNtp) + "\r\n"));
#endif
        // End Debug
        ESP.rtcUserMemoryWrite(RTCaddr_hour, &hourNtp, sizeof(hourNtp));
        ESP.rtcUserMemoryWrite(RTCaddr_minute, &minuteNtp, sizeof(minuteNtp));
        ESP.rtcUserMemoryWrite(RTCaddr_seconds, &secondsNtp, sizeof(secondsNtp));

        // need considering offset when setting
        int32_t timeoutMs_noOffset = TIMER_INTERVAL_60S - secondsNtp * 1000;
        int32_t timeoutMs_withOffset = timeoutMs_noOffset + offsetMsPerCalInterval * (timeoutMs_noOffset) / (60 * 1000 * TIMER_CALI_INTERVAL_MIN);
#ifdef DEBUG
        Serial.print("One Shot timeoutMs_noOffset,  millis() =   ");
        Serial.println(timeoutMs_noOffset);
        Serial.print("One Shot timeoutMs_withOffset,  millis() =   ");
        Serial.println(timeoutMs_withOffset);
#endif
        ISR_Timer.setTimeout((uint32_t)timeoutMs_withOffset, TimeCalOneShotHandler);

        runStatus = ClockSyncMode;

        // Rotate moto fast to target.
        int32_t minuteDiff = (int32_t)(hourNtp * 60 + minuteNtp) - (int32_t)(currHourMoto * 60 + currMinuteMoto);

        // 24H vs 12H
        if (minuteDiff < 0)
            minuteDiff += 24 * 60;
        if (minuteDiff > 60 * 12)
            minuteDiff -= 12 * 60;

#ifdef DEBUG
        Serial.println("Moto need rotate: " + String(minuteDiff) + " minutes");
#endif
        RotateFast((minuteDiff * STEPS_PER_ROTATION) / 60);
        currHourMoto = hourNtp;
        currMinuteMoto = minuteNtp;
    }
    else
    {
        runStatus = FreeRun;
    }
}


void TimerSyncWithNtp()
{

    unsigned long clockPrevious = 0, clockAfter = 0; 
    unsigned long ntpSyncTimeMs = 0;
    uint32_t minute, hour, seconds;
    uint32_t hourNtp, minuteNtp, secondsNtp;
    int32_t offsetSeconds, alignPointDiff;
    uint16_t syncRetryCount = 0;
    bool syncSuccessFlag = false;

    // check WiFi status again
    if (WiFi.status() != WL_CONNECTED)
    {
        if (!ConnectingWiFi(true))
        {
            clockSyncFailureCount++;
            isSyncTriggerred = false;
#ifdef DEBUG
            Serial.print(("WiFi Status Error, skip time sync of this round."));
#endif
            return;
        }
    }

    do
    {
        clockPrevious = millis();
        syncSuccessFlag = timeClient.update();
        if (!syncSuccessFlag)
            continue;
        hourNtp = timeClient.getHours();
        minuteNtp = timeClient.getMinutes();
        secondsNtp = timeClient.getSeconds();

        // NTP lib seems will return strange value sometimes
        if (hourNtp > 24 || minuteNtp > 60 || secondsNtp > 60)
        {
            syncSuccessFlag = false;
            continue;
        }
        
        clockAfter = millis();
        if (syncSuccessFlag)
            break;
    } while (++syncRetryCount <= 3);

    if (syncSuccessFlag)
    {
        ISR_Timer.disable(timerClockId);
        ISR_Timer.deleteTimer(timerClockId);
        ISR_Timer.disable(timerSyncId);
        ISR_Timer.deleteTimer(timerSyncId);

        // Calc difference caused by different point
        ntpSyncTimeMs = clockAfter / 2 + clockPrevious / 2;
        if(ntpSyncTimeMs > currSysTimeMs)
        {
            alignPointDiff = ((ntpSyncTimeMs - currSysTimeMs) / 1000);
        }
        else if(currSysTimeMs > ntpSyncTimeMs)
        {
            alignPointDiff = -1 * ((currSysTimeMs - ntpSyncTimeMs) / 1000);
        }
        else{
            alignPointDiff = 0;
        }

        CheckTimeAndRotateMoto();   // possible that time just change before timer stoped.
        ESP.rtcUserMemoryRead(RTCaddr_hour, &hour, sizeof(hour));
        ESP.rtcUserMemoryRead(RTCaddr_minute, &minute, sizeof(minute));
        ESP.rtcUserMemoryRead(RTCaddr_seconds, &seconds, sizeof(seconds));

        // Here NPT and Timer should be aligned, calculate offset.
        uint32_t ntpTime = hourNtp * 3600 + minuteNtp * 60 + secondsNtp;
        uint32_t time = hour * 3600 + minute * 60 + seconds;
        offsetSeconds = (int32_t)time - (int32_t)ntpTime + alignPointDiff;

        // Considering 00:00, change fast/slow logic
        int32_t halfDayInSeconds = 12 * 60 * 60;
        if (offsetSeconds > halfDayInSeconds)
            offsetSeconds -= halfDayInSeconds * 2;
        else if (offsetSeconds < -1 * halfDayInSeconds)
            offsetSeconds += halfDayInSeconds * 2;
            
        if (clockSyncFailureCount <= 4)
        {
            offsetMsPerCalInterval += (offsetSeconds * 1000) / (clockSyncFailureCount + 1);
        }
        else
        {
            offsetMsPerCalInterval = 0;
        }

// Debug info
#ifdef DEBUG
        Serial.print(("Starting NTP Calibration, millis() = "));
        Serial.println(clockPrevious);
        Serial.print(("NTP millis() = :  "));
        Serial.println(clockAfter);
        Serial.print(("Sys TIme:  " + String(hour) + ":" + String(minute) + ":" + String(seconds) + "\r\n"));
        Serial.print(("NTP TIme:  " + String(hourNtp) + ":" + String(minuteNtp) + ":" + String(secondsNtp) + "\r\n"));
#endif
        // End Debug
        ESP.rtcUserMemoryWrite(RTCaddr_hour, &hourNtp, sizeof(hourNtp));
        ESP.rtcUserMemoryWrite(RTCaddr_minute, &minuteNtp, sizeof(minuteNtp));
        ESP.rtcUserMemoryWrite(RTCaddr_seconds, &secondsNtp, sizeof(secondsNtp));

        int32_t timeoutMs_noOffset = TIMER_INTERVAL_60S - secondsNtp * 1000;
        int32_t timeoutMs_withOffset = timeoutMs_noOffset + offsetMsPerCalInterval * (timeoutMs_noOffset) / (60 * 1000 * TIMER_CALI_INTERVAL_MIN);
#ifdef DEBUG
        Serial.print("One Shot timeoutMs_noOffset,  millis() =   ");
        Serial.println(timeoutMs_noOffset);
        Serial.print("One Shot timeoutMs_withOffset,  millis() =   ");
        Serial.println(timeoutMs_withOffset);
#endif
        ISR_Timer.setTimeout((uint32_t)timeoutMs_withOffset, TimeCalOneShotHandler);

        clockSyncFailureCount = 0;
    }
    else
    {
        clockSyncFailureCount++;
    }
    isSyncTriggerred = false;
}

