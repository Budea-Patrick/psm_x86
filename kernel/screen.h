#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "main.h"
#include "logging.h"
#include "scancodes.h"
#include "string.h"

#define MAX_LINES       25
#define MAX_COLUMNS     80
#define MAX_OFFSET      2000 //25 lines * 80 chars
#define COLOR_GREEN     10

#define BRIGHT_WHITE_COLOR          0xF
#define YELLOW_COLOR                0xE
#define BRIGHT_MAGENTA_COLOR        0xD
#define BRIGHT_RED_COLOR            0xC
#define BRIGHT_CYAN_COLOR           0xB
#define BRIGHT_GREEN_COLOR          0xA        
#define BRIGHT_BLUE_COLOR           0x9
#define GRAY_COLOR                  0x8
#define WHITE_COLOR                 0x7
#define BROWN_COLOR                 0x6
#define MAGENTA_COLOR               0x5
#define RED_COLOR                   0x4
#define CYAN_COLOR                  0x3
#define GREEN_COLOR                 0x2
#define BLUE_COLOR                  0x1
#define BLACK_COLOR                 0x0

#pragma pack(push)
#pragma pack(1)
typedef struct _SCREEN
{
    char c;
    BYTE color;
}SCREEN, * PSCREEN;
#pragma pack(pop)

void HelloBoot();

void SetColor(BYTE Color);
void ClearScreen(void* VideoMemoryBuffer, DWORD BufferSize, int* CursorPosition);
void PutChar(char C, int Pos);
void PutString(char* String, int Pos);
void PutStringLine(char* String, int Line);
void ScreenDisplay(char* logBuffer, BYTE color);
void DisplayKeyPress(BYTE scancode, int isExtended);
void CLIProcessCommand(char* command);
void MoveCursorToNextLine();
void EnterEditMode();

#endif // _SCREEN_H_