#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "main.h"
#include "string.h"
#include "screen.h"

#define LOG_BUF_MAX_SIZE	512

void InitLogging();

void Log(char* Message);

void LogSerial(int DisplayToTerminal, char* FormatBuffer, ...);

#endif // _LOGGING_H_