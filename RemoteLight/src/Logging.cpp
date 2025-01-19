#include "Logging.h"

void printLog(const char* level, const char* path, const char* fileName, int line, const char* message, ...) {
  #if DEBUG
    if (!level || !path || !fileName || !message) {
      return;
    }

    char* pos = strrchr(path, '/');
    char *pathFile = (char *)malloc(20 * sizeof(char));
    int found = pos - path + 1;
    int i = 0;
    while (path[found] != '\0') {
        pathFile[i++] = path[found++];
    }
    pathFile[i] = '\0';

    char buffer[256];
    va_list args;
    va_start(args, message);
    unsigned long time = millis();
    snprintf(buffer, sizeof(buffer), "%lu \t[%s][%s:%s():%d]: ",time, level, pathFile, fileName, line);
    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), message, args);
    va_end(args);

    Serial.println(buffer);

    free(pathFile);
  #else
    return;
  #endif
}