#include "FsUtility.h"

const char *fsName = "LittleFS";
FS *fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();

String unsupportedFiles = String();
static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

void InitializeFileSystem()
{

    //****** 文件系统初始化 ******
    fileSystemConfig.setAutoFormat(false);
    LittleFS.setConfig(fileSystemConfig);
    if (LittleFS.begin())
        Serial.println("LittleFS 启动成功");
    else
    {
        Serial.println("LittleFS 未能成功启动");
    }
}