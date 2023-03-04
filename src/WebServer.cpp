#include "WebServer.h"
#include "WifiUtility.h"
#include <LittleFS.h>

ESP8266WebServer server(80);         // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
ESP8266HTTPUpdateServer httpUpdater; // 建立httpOTA对象
bool isServerOn = false;

void InitializeWebServer()
{
    server.on("/", WebRootHandler);                    // 处理主页请求
    server.on("/Read_scanWifi", ScanWifiHandler);      // ajax 扫描周边wifi
    server.on("/Wifi", HTTP_POST, WebRootPostHandler); // 处理主页wifi表单
    server.onNotFound(WebNotFoundHandler);             // 处理网页url请求和404

    // 配置http更新服务为为&server
    httpUpdater.setup(&server, "/update", "Jones", "3333");
    // 启动WebServer
    server.begin();
    isServerOn = true;
    Serial.println("Web Server Started");
}

void StopWebServer()
{
    if(isServerOn)
        server.stop();
    isServerOn = false;
    Serial.println("Web Server Stopped");
}

void ScanWifiHandler() // 扫描周边wifi
{
    String web_message;
    int8_t n;
    n = WiFi.scanNetworks(); // 扫描wifi
    if (n > 0)
    {
        web_message = "<label for='sel1'>周边网络:</label>";
        web_message += "<select class='form-control' id='sel1' name='ssid'>";
        for (uint8_t i = 0; i < n; i++)
        {
            web_message += "<option>" + WiFi.SSID(i) + "</option>";
        }
        web_message += "</select>";
    }
    else
    {
        web_message = F("<label for='sel1'>周边网络:</label>");
        web_message += F("<select class='form-control' id='sel1'>");
        web_message += F("<option >无扫描结果</option>");
        web_message += F("</select>");
    }
    server.send(200, "text/plain", web_message);
}

void WebRootHandler() // 处理网站根目录"/"的访问请求
{
    String path = "/MAIN.html";
    String contentType = GetContentType(path); // 获取文件类型
    String pathWithGz = path + ".gz";

    Serial.println("WebRoot Handler Called");
    if (LittleFS.exists(path)) // 如果访问的文件可以在LittleFS中找到
    {
        File file = LittleFS.open(path, "r"); // 则尝试打开该文件
        server.streamFile(file, contentType); // 并且将该文件返回给浏览器
        file.close();                         // 并且关闭文件
    }
    else if (LittleFS.exists(pathWithGz))
    {
        File file = LittleFS.open(pathWithGz, "r");
        server.streamFile(file, contentType);
        file.close();
    }
    else
    {
        server.send(200, "text/html", SendHTMLMainErrorPage());
    }
}

String GetContentType(String filename)
{
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}
void WebRootPostHandler()
{
    String wifi_xiaoxi = "WIFI连接中，可能会断开热点";
    // char *strcpy(char *dest, const char *src) 把 src 所指向的字符串复制到 dest。
    // arg(name) —— 根据请求key获取请求参数的值
    // arg(index) —— 获取第几个请求参数的值
    /*Serial.print("WiFi表单-参数个数 "); Serial.println(server.args());
      Serial.print("WiFi表单-参数0名称 "); Serial.println(server.argName(0));
      Serial.print("WiFi表单-参数0数值 "); Serial.println(server.arg("plain"));*/
    if (server.hasArg("ssid")) // 查询是否存在某个参数
    {
        strcpy(sta_ssid, server.arg("ssid").c_str()); // 保存参数
        if (strlen(sta_ssid) == 0)
            wifi_xiaoxi = "WIFI名称不能为空"; // 检查WIFI名称长度为0时退出
    }
    else
        wifi_xiaoxi = "未找到参数'ssid'";

    // 查询是否存在某个参数
    if (server.hasArg("password"))
        strcpy(sta_password, server.arg("password").c_str()); // 保存参数
    else
        wifi_xiaoxi += "  未找到参数'password'";

    // server.send(200, "text/html", sendHTML_return_home(wifi_xiaoxi));
    server.send(200, "text/plain", wifi_xiaoxi);

    if (wifi_xiaoxi == "WIFI连接中，可能会断开热点")
    {
        WiFi.disconnect();
        // overtime = millis(); //重置配网超时时间
        // peiwangInitStete = 0; //重置配网初始化
        wifiStatus = Connecting; // 发送连接指令
        // sta_count = 0;
        Serial.println("Change wifi mode to Connecting.");
    }
}
void WebNotFoundHandler()
{
    // Serial.println("NotFonund Handler");
}

String SendHTMLMainErrorPage() // 文件固件丢失提示界面，备用页面#include <u8g2_fonts.h>
{
    String webpage_html;
    webpage_html += F("<!DOCTYPE html>");
    webpage_html += F("<html lang='zh-CN'>");
    webpage_html += F("<head>");
    webpage_html += F("<title>悬浮时钟</title>");
    webpage_html += F("<meta charset='utf-8'>");
    webpage_html += F("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    webpage_html += F("</head>");
    webpage_html += F("<body>");
    webpage_html += F("<h3>糟糕！无法找到该页面，可能原因如下：</h3>");
    webpage_html += F("<p>1.文件系统未配置或网页丢失<br>2.文件系统不兼容，应为LittleFS<br>3.没有该页面<br>4.文件系统与程序固件不匹配<p><br>");
    webpage_html += F("</body></html>");

    return webpage_html;
}