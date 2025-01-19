#ifndef LOGGING_H
#define LOGGING_H

#define DEBUG 1
#include <Arduino.h>
#include <string>

#define LOGE(message, ...) printLog("ERROR", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGW(message, ...) printLog("WARN", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGI(message, ...) printLog("INFO", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGD(message, ...) printLog("DEBUG", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGV(message, ...) printLog("VERBOSE", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)

void printLog(const char* level, const char* path, const char* fileName, int line, const char* message, ...);

#endif // LOGGING_H