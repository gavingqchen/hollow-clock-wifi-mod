#ifndef _WIFI_UTILITY_H
#define _WIFI_UTILITY_H
#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>

#define WIFI_CONNECT_TIMEOUT_MIN        5L



extern IPAddress dns1;
extern IPAddress dns2;
//****** AP设置
extern const char *ap_ssid;
extern const char *ap_password;
extern IPAddress local_IP;
extern IPAddress gateway;
extern IPAddress subnet;
extern char sta_ssid[32];
extern char sta_password[64];

enum WifiStatus{
    NotSet = 0,
    SsidNotFound = 1,
    NotConnected = 3,
    Connected = 4,
    Connecting = 5,
    UserQuit = 6,
};
extern WifiStatus wifiStatus;

void InitializeWifi();
void WifiConfiguration();
bool ConnectingWiFi(bool isAutoConnect);
void InitWifiApStaMode();
void InitWifiApMode();
bool ScanNetworks(String currentSSID);

#endif