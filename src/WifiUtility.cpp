#include "WifiUtility.h"
#include "EepromUtility.h"
#include "WebServer.h"
#include "Utility.h"

//****** STA设置
char sta_ssid[32] = {0};
char sta_password[64] = {0};
IPAddress dns1(114, 114, 114, 114);
IPAddress dns2(114, 114, 115, 115);
//****** AP设置
const char *ap_ssid = "ESP8266 Hollow Clock";
const char *ap_password = "333333333"; // 无密码则为开放式网络 9个3
IPAddress local_IP(192, 168, 3, 3);
IPAddress gateway(192, 168, 3, 3);
IPAddress subnet(255, 255, 255, 0);
WifiStatus wifiStatus = NotSet;

void InitializeWifi()
{

    WiFi.mode(WIFI_OFF); //  设置工作模式
    wifiStatus = NotSet;
    WiFi.persistent(true);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);

    WiFi.setOutputPower(18);
    eepUserSet.outputPower = 18.0;
    InitWifiApStaMode();
}

void WifiConfiguration()
{
    uint32_t timeOutMs = WIFI_CONNECT_TIMEOUT_MIN * 60 * 1000; 
    while (millis() < timeOutMs)
    {
        // Only run when under NotSet mode, which means on bootup
        if (WiFi.SSID().length() > 0 && wifiStatus == NotSet)
        {
#ifdef DEBUG
            Serial.println("Connecting Previous SSID: " + WiFi.SSID());
#endif
            ConnectingWiFi(true);
        }
        switch (wifiStatus)
        {
        case NotSet:
        case NotConnected:
        case SsidNotFound:
            // Display configuration page and set webserver ON
            if (!isServerOn)
            {
                WiFi.mode(WIFI_AP_STA);
                InitializeWebServer();
            }
            break;
        case Connected:
            break;
        case Connecting:
            // This would be set in webserver
            ConnectingWiFi(false);
            break;
        case UserQuit:
            break;
        default:
            break;
        }

        if (isServerOn)
            server.handleClient();
    }
    if (WiFi.status() != WL_CONNECTED)
        runStatus =  WiFiConnected;
    else
        runStatus =  FreeRun;
}

bool ConnectingWiFi(bool isAutoConnect)
{
    WiFi.persistent(true);  // 需要保存
    WiFi.mode(WIFI_STA);

    if(isAutoConnect)
        WiFi.begin();
    else
        WiFi.begin(sta_ssid, sta_password);
    
    Serial.println( "Starting WiFi Connecting...");

    int sta_count = 0;
    while (WiFi.isConnected() == false && sta_count < 15) // 尝试连接
    {
        ESP.wdtFeed(); // 喂狗
        server.handleClient();
        delay(1000);
        sta_count++;
        if (sta_count >= 15)
        {
            wifiStatus = NotConnected; // wifi连接失败
        }
    }
    if (WiFi.isConnected())
    {
        wifiStatus = Connected;
        StopWebServer();
        WiFi.setOutputPower(eepUserSet.outputPower); // 设置发射功率
        WiFi.setAutoConnect(1);                      // 设置自动连接wifi
        WiFi.setSleepMode(WIFI_NONE_SLEEP);
        WiFi.config(0, 0, 0, dns1, dns2);
        WiFi.begin();
        Serial.println( "WiFi Connected, SSID: " + WiFi.SSID());
    }
    else
    {
        Serial.println("WiFi Connecting Failed, SSID: " + WiFi.SSID());
        wifiStatus = NotConnected;
        // StopWebServer();
        // WiFi.mode(WIFI_OFF);
        // WiFi.persistent(true);
        WiFi.setSleepMode(WIFI_NONE_SLEEP);
        WiFi.setOutputPower(eepUserSet.outputPower);
        // InitWifiApStaMode();
    }
    return WiFi.isConnected();
}


/*** 初始化AP配置 ***/
void InitWifiApStaMode()
{
    WiFi.mode(WIFI_AP_STA);                                 // 设置工作模式
    WiFi.config(0, 0, 0, dns1, dns2);                       // sta模式的dns
    WiFi.softAPConfig(local_IP, gateway, subnet);           // ap的网络参数
    WiFi.softAP(ap_ssid, ap_password, random(1, 14), 0, 1); // ap的名称和密码
    WiFi.begin();
}

void InitWifiApMode()
{
    WiFi.softAPConfig(local_IP, gateway, subnet);           // ap的网络参数
    WiFi.softAP(ap_ssid, ap_password, random(1, 14), 0, 1); // ap的名称、密码、信道、是否隐藏、同时连接数
    WiFi.mode(WIFI_AP);                                     // 设置工作模式
}

bool ScanNetworks(String currentSSID)
{
    bool scanState = false;          // 扫描状态，0-扫描到 1-无
    uint8_t n = WiFi.scanNetworks(); // 扫描wifi
    if (n == 0)
        scanState = false; // 扫描不到开启ap模式
    else
    {
        for (uint8_t i = 0; i < n; i++)
        {
            // Serial.println(WiFi.SSID(i));
            if (strcmp(currentSSID.c_str(), WiFi.SSID(i).c_str()) == 0)
                scanState = true;
            else if (scanState != 1 && strcmp(currentSSID.c_str(), WiFi.SSID(i).c_str()) != 0)
                scanState = false;
        }
    }
    WiFi.scanDelete(); // 删除扫描的结果
    return scanState;
}