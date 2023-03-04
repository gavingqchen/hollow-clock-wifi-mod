#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266HTTPClient.h>

extern ESP8266WebServer server;  
extern bool isServerOn;

String GetContentType(String filename);
void InitializeWebServer();
void WebRootHandler();
String GetContentType(String filename);
void WebNotFoundHandler();
void ScanWifiHandler();
void WebRootPostHandler();
String SendHTMLMainErrorPage();
void StopWebServer();

#endif