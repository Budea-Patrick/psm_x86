#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "screen.h"
#include "cmos.h"
#include "ata.h"

#define MAX_COMMAND_LENGTH 256
void CLIProcessCommand(char* command);
void PrintMBR();

#endif // !_COMMANDS_H
