# 悬浮时钟 WiFi 同步版

## 说明
悬浮时钟WiFi同步版。使用ESP8266进行控制，以取代原作者的Arduino Nano。
原作者项目地址: [Hollow Clock4](https://www.instructables.com/Hollow-Clock-4/)
电机控制部分重用了原作者的代码。
网页登录服务参考和重用了[甘草酸不酸](https://gitee.com/Lichengjiez/weather-ink-screen)墨水屏的代码。
参考了Github[nikzin96](https://github.com/nikzin96/modified_HollowClock4)WiFi版本代码。

重写了网络同步代码，采用ISR Timer进行时间计时。[ESP8266TimerInterrupt]https://github.com/khoih-prog/ESP8266TimerInterrupt

## 说明
1. 默认开启时时钟处于12：00 位置。
2. 上电后自动尝试连接上一次保存的WiFi热点。若连接失败则打开网页服务。
3. 用户可在5分钟内连接热点 "ESP8266 Hollow Clock", 密码 "333333333"，然后登录网页192.168.3.3进行网络配置。期间时钟处于FreeRun模式走时。
4. 若5分钟内连接并且同步时间成功，则时钟自动快速走时到当前网络时间，并开启定期自动同步功能。
5. 若WiFi未连接或者连接后无法联网同步，则时钟将保持FreeRun模式。

## 开发环境
- VsCode + Platform IO

## 项目链接
[Gitee](https://gitee.com/guanqunchen/hollow-clock_-wifi-mod)
[GitHub](https://gitee.com/guanqunchen/hollow-clock_-wifi-mod)


