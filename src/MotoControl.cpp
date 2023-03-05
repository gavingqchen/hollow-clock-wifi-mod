#include <Arduino.h>
#include "MotoControl.h"
#include "Utility.h"

int port[4] = {D8, D7, D6, D5};
// wait for a single step of stepper
int delaytime = 2;
uint32_t currHourMoto = 0;
uint32_t currMinuteMoto = 0;
// sequence of stepper motor control
int seq[8][4] = {
    {LOW, HIGH, HIGH, LOW},
    {LOW, LOW, HIGH, LOW},
    {LOW, LOW, HIGH, HIGH},
    {LOW, LOW, LOW, HIGH},
    {HIGH, LOW, LOW, HIGH},
    {HIGH, LOW, LOW, LOW},
    {HIGH, HIGH, LOW, LOW},
    {LOW, HIGH, LOW, LOW}};

void MotoInitialize()
{
    pinMode(port[0], OUTPUT);
    pinMode(port[1], OUTPUT);
    pinMode(port[2], OUTPUT);
    pinMode(port[3], OUTPUT);

    ESP.rtcUserMemoryRead(RTCaddr_hour, &currHourMoto, sizeof(currHourMoto));
    ESP.rtcUserMemoryRead(RTCaddr_minute, &currMinuteMoto, sizeof(currMinuteMoto));
    Rotate(-20); // for approach run
    Rotate(20);  // approach run without heavy load
}

void CheckTimeAndRotateMoto()
{
    uint32_t minute, hour;
    ESP.rtcUserMemoryRead(RTCaddr_hour, &hour, sizeof(hour));
    ESP.rtcUserMemoryRead(RTCaddr_minute, &minute, sizeof(minute));
    if (!(hour == currHourMoto && minute == currMinuteMoto))
    {
        if (hour == 0)
            hour = 24;
        if (currHourMoto == 0)
            currHourMoto = 24;

        int32_t minuteDiff = (int32_t)(hour * 60 + minute) - (int32_t)(currHourMoto * 60 + currMinuteMoto);
        if (minuteDiff > 0)
        {
            Rotate((minuteDiff * STEPS_PER_ROTATION) / 60);

            if (hour == 24)
                hour = 0;
#ifdef DEBUG
            Serial.println("Moto need rotate: " + String(minuteDiff) + " minutes");
            Serial.println("Current Time: " + String(hour) + ":" + String(minute));
#endif
            currHourMoto = hour;
            currMinuteMoto = minute;
        }
        else
        {
            // Just wait until need rotate.
        }
    }
}

void Rotate(int step)
{ // original function from shiura
    static int phase = 0;
    int i, j;
    int delta = (step > 0) ? 1 : 7;
    int dt = 20;

    step = (step > 0) ? step : -step;
    for (j = 0; j < step; j++)
    {
        phase = (phase + delta) % 8;
        for (i = 0; i < 4; i++)
        {
            digitalWrite(port[i], seq[phase][i]);
        }
        delay(dt);
        if (dt > delaytime)
            dt--;
    }
    // power cut
    for (i = 0; i < 4; i++)
    {
        digitalWrite(port[i], LOW);
    }
}

void RotateFast(int step)
{ // this is just to rotate to the current time faster, when clock is started
    static int phase = 0;
    int i, j;
    int delta = (step > 0) ? 1 : 7;
    int dt = 1;

    step = (step > 0) ? step : -step;
    for (j = 0; j < step; j++)
    {
        phase = (phase + delta) % 8;
        for (i = 0; i < 4; i++)
        {
            digitalWrite(port[i], seq[phase][i]);
        }
        delay(dt);
        if (dt > delaytime)
            dt--;
    }
    // power cut
    for (i = 0; i < 4; i++)
    {
        digitalWrite(port[i], LOW);
    }
}