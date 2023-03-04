#include <Arduino.h>
#include "MotoControl.h"
#include "Utility.h"

int port[4] = {D8, D7, D6, D5};
// wait for a single step of stepper
int delaytime = 2;
uint32_t currHour = 0;
uint32_t currMinute = 0;
// sequence of stepper motor control
int seq[8][4] = {
  {  LOW, HIGH, HIGH,  LOW},
  {  LOW,  LOW, HIGH,  LOW},
  {  LOW,  LOW, HIGH, HIGH},
  {  LOW,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW,  LOW},
  { HIGH, HIGH,  LOW,  LOW},
  {  LOW, HIGH,  LOW,  LOW}
};


void MotoInitialize()
{
    pinMode(port[0], OUTPUT);
    pinMode(port[1], OUTPUT);
    pinMode(port[2], OUTPUT);
    pinMode(port[3], OUTPUT);

    ESP.rtcUserMemoryRead(RTCaddr_hour, &currHour, sizeof(currHour));
    ESP.rtcUserMemoryRead(RTCaddr_minute, &currMinute, sizeof(currMinute));
    Rotate(-20); // for approach run
    Rotate(20);  // approach run without heavy load
}

void CheckTimeAndRotateMoto()
{
    uint32_t minute, hour;
    ESP.rtcUserMemoryRead(RTCaddr_hour, &hour, sizeof(hour));
    ESP.rtcUserMemoryRead(RTCaddr_minute, &minute, sizeof(minute));

    if (hour == 0)
        hour = 24;
    if (currHour == 0)
        currHour = 24;
    if (hour * 60 + minute > currHour * 60 + minute)
    {
        uint32_t minuteDiff = hour * 60 + minute -(currHour * 60 + minute);
        Rotate((minuteDiff * STEPS_PER_ROTATION) / 60);

        if(hour == 24) hour = 0;
        ESP.rtcUserMemoryWrite(RTCaddr_hour, &hour, sizeof(hour));
        ESP.rtcUserMemoryWrite(RTCaddr_minute, &minute, sizeof(minute));
#ifdef DEBUG
        Serial.println("Moto need rotate: " + String(minuteDiff) + " minutes");
        Serial.println("Current Time: " + String(hour) + ":" + String(minute));
#endif
    }
    else
    {
        // Just wait until need rotate.
    }
}

void Rotate(int step) { // original function from shiura
  static int phase = 0;
  int i, j;
  int delta = (step > 0) ? 1 : 7;
  int dt = 20;

  step = (step > 0) ? step : -step;
  for(j = 0; j < step; j++) {
    phase = (phase + delta) % 8;
    for(i = 0; i < 4; i++) {
      digitalWrite(port[i], seq[phase][i]);
    }
    delay(dt);
    if(dt > delaytime) dt--;
  }
  // power cut
  for(i = 0; i < 4; i++) {
    digitalWrite(port[i], LOW);
  }
}

void RotateFast(int step) { // this is just to rotate to the current time faster, when clock is started
  static int phase = 0;
  int i, j;
  int delta = (step > 0) ? 1 : 7;
  int dt = 1;

  step = (step > 0) ? step : -step;
  for(j = 0; j < step; j++) {
    phase = (phase + delta) % 8;
    for(i = 0; i < 4; i++) {
      digitalWrite(port[i], seq[phase][i]);
    }
    delay(dt);
    if(dt > delaytime) dt--;
  }
  // power cut
  for(i = 0; i < 4; i++) {
    digitalWrite(port[i], LOW);
  }
}