#ifndef _FS_UTILITY_
#define _FS_UTILITY_
#include <LittleFS.h> // 官方要求的新文件系统库  #include "FS.h"未来将不会得官方支持，已弃用

extern LittleFSConfig fileSystemConfig;

void InitializeFileSystem();


#endif