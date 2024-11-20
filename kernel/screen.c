#include "screen.h"
#define MAX_HISTORY 10
#define MAX_COMMAND_LENGTH 256
#define MAX_EDIT_LENGTH 1024

char commandHistory[MAX_HISTORY][MAX_COMMAND_LENGTH];
int historyCount = 0;
int historyIndex = -1;
int currentHistoryIndex = -1;

 PSCREEN gVideo = (PSCREEN)(0x000B8000);
 int CURSOR_P = 0;

 char commandBuffer[MAX_COMMAND_LENGTH];
 int commandLength = 0;

 int isInEditMode = 0;
 char editBuffer[MAX_EDIT_LENGTH];
 int editLength = 0;
 int editCursorPos = 0;
 int LAST_WRITTEN_CURSOR = 0;

void CursorMove(int row, int col)
{
    unsigned short location = (row * MAX_COLUMNS) + col;       /* Short is a 16-bit type; the formula is used here */

    // Cursor Low port
    __outbyte(0x3D4, 0x0F);                                    // Sending the cursor low byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)(location & 0xFF));

    // Cursor High port
    __outbyte(0x3D4, 0x0E);                                    // Sending the cursor high byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)((location >> 8) & 0xFF)); // Char is an 8-bit type
}

void CursorPosition(int pos)
{
    int row, col;

    if (pos > MAX_OFFSET)
    {
        pos = pos % MAX_OFFSET;
    }

    row = pos / MAX_COLUMNS;
    col = pos % MAX_COLUMNS;

    CursorMove(row, col);
}

void MoveCursorToNextLine() {
    CURSOR_P = (CURSOR_P / MAX_COLUMNS + 1) * MAX_COLUMNS;

    if (CURSOR_P >= MAX_OFFSET) {
        for (int i = 0; i < MAX_OFFSET - MAX_COLUMNS; i++) {
            gVideo[i] = gVideo[i + MAX_COLUMNS];
        }

        for (int i = MAX_OFFSET - MAX_COLUMNS; i < MAX_OFFSET; i++) {
            gVideo[i].color = BRIGHT_GREEN_COLOR;
            gVideo[i].c = ' ';
        }

        CURSOR_P -= MAX_COLUMNS;
    }

    CursorPosition(CURSOR_P);
}

void HelloBoot()
{
    int i, len;
    char boot[] = "Hello Boot! Greetings from C...";

    len = 0;
    while (boot[len] != 0)
    {
        len++;
    }

    for (i = 0; (i < len) && (i < MAX_OFFSET); i++)
    {
        gVideo[i].color = 10;
        gVideo[i].c = boot[i];
    }
    CursorPosition(i);
}

void EnterEditMode() {
    isInEditMode = 1;
    ClearScreen(NULL, 0, NULL);
}

void ClearScreen(void* VideoMemoryBuffer, DWORD BufferSize, int* CursorPosition) {
    unsigned int i;

    if (VideoMemoryBuffer != NULL && BufferSize >= MAX_OFFSET * sizeof(PSCREEN)) {
        PSCREEN src = gVideo;
        PSCREEN dest = (PSCREEN)VideoMemoryBuffer;
        for (i = 0; i < MAX_OFFSET; i++) {
            dest[i].c = src[i].c;
            dest[i].color = src[i].color;
        }
    }

    if (CursorPosition != NULL) {
        *CursorPosition = CURSOR_P;
    }

    for (i = 0; i < MAX_OFFSET; i++) {
        gVideo[i].color = BRIGHT_GREEN_COLOR;
        gVideo[i].c = ' ';
    }


    CURSOR_P = 0;
    CursorMove(0, 0);
}

void ScreenDisplay(char* logBuffer, BYTE color)
{
    unsigned int len = 0;
    while (len < MAX_OFFSET && logBuffer[len] != 0) {
        len++;
    }

    unsigned int i;
    for (i = 0; i < len; i++) {
        if (logBuffer[i] == '\n') {
            MoveCursorToNextLine();
            continue;
        }

        gVideo[CURSOR_P].color = color;
        gVideo[CURSOR_P].c = logBuffer[i];
        CURSOR_P = CURSOR_P + 1;
    }

    CursorPosition(CURSOR_P);
}

void AddToHistory(const char* command) {
    if (historyCount < MAX_HISTORY) {
        strcpy(commandHistory[historyCount], command);
        historyCount++;
    }
    else {
        for (int i = 1; i < MAX_HISTORY; i++) {
            strcpy(commandHistory[i - 1], commandHistory[i]);
        }
        strcpy(commandHistory[MAX_HISTORY - 1], command);
    }
    currentHistoryIndex = historyCount;
}

void ClearCurrentLine() {
    int row = CURSOR_P / MAX_COLUMNS;
    int startPos = row * MAX_COLUMNS;

    for (int i = 0; i < MAX_COLUMNS; i++) {
        gVideo[startPos + i].c = ' ';
        gVideo[startPos + i].color = BRIGHT_GREEN_COLOR;
    }

    CURSOR_P = startPos;
    CursorPosition(CURSOR_P);
}

void DisplayKeyPress(BYTE scancode, int isExtended) {
    KEYCODE key;

    if (isExtended != 0) {
        if (scancode < sizeof(_kkybrd_scancode_ext) / sizeof(_kkybrd_scancode_ext[0])) {
            key = _kkybrd_scancode_ext[scancode];
        }
        else {
            key = KEY_UNKNOWN;
        }
    }
    else {
        if (scancode < sizeof(_kkybrd_scancode_std) / sizeof(_kkybrd_scancode_std[0])) {
            key = _kkybrd_scancode_std[scancode];
        }
        else {
            key = KEY_UNKNOWN;
        }
    }

    if (isInEditMode) {
        if (key == KEY_ESCAPE) {
            isInEditMode = 0;
            ClearScreen(NULL, 0, NULL);

            editBuffer[0] = '\0';
            editLength = 0;
            editCursorPos = 0;
            CURSOR_P = 0;
        }
        else if (key == KEY_BACKSPACE) {
            if (editCursorPos > 0) {
                editCursorPos--;
                editLength--;
                for (int i = editCursorPos; i < editLength; i++) {
                    editBuffer[i] = editBuffer[i + 1];
                }
                editBuffer[editLength] = '\0';

                ClearCurrentLine();
                ScreenDisplay(editBuffer, BRIGHT_WHITE_COLOR);
                CURSOR_P = editCursorPos;
                CursorPosition(CURSOR_P);
            }
        }
        else if (key == KEY_LEFT) {
            if (editCursorPos > 0) {
                editCursorPos--;
                CURSOR_P--;
                CursorPosition(CURSOR_P);
            }
        }
        else if (key == KEY_RIGHT) {
            if (editCursorPos < editLength) {
                editCursorPos++;
                CURSOR_P++;
                CursorPosition(CURSOR_P);
            }
        }
        else if (key == KEY_UP) {
            if (CURSOR_P >= MAX_COLUMNS) {
                CURSOR_P -= MAX_COLUMNS;
                CursorPosition(CURSOR_P);
            }
        }
        else if (key == KEY_DOWN) {
            if (CURSOR_P + MAX_COLUMNS < MAX_OFFSET && CURSOR_P + MAX_COLUMNS < LAST_WRITTEN_CURSOR) {
                CURSOR_P += MAX_COLUMNS;
                CursorPosition(CURSOR_P);
            }
        }
        else if (editLength < MAX_EDIT_LENGTH - 1) {
            editBuffer[editCursorPos++] = key;
            editLength++;
            editBuffer[editLength] = '\0';

            gVideo[CURSOR_P].color = BRIGHT_WHITE_COLOR;
            gVideo[CURSOR_P].c = key;
            CURSOR_P++;
            CursorPosition(CURSOR_P);

            if (CURSOR_P > LAST_WRITTEN_CURSOR) {
                LAST_WRITTEN_CURSOR = CURSOR_P;
            }
        }
        return;
    }

    if (key == KEY_RETURN) {
        commandBuffer[commandLength] = '\0';
        CLIProcessCommand(commandBuffer);
        AddToHistory(commandBuffer);
        commandLength = 0;
    }
    else if (key == KEY_BACKSPACE) {
        if (commandLength > 0) {
            commandLength--;

            if (CURSOR_P > 0) {
                CURSOR_P--;
            }

            gVideo[CURSOR_P].color = BRIGHT_GREEN_COLOR;
            gVideo[CURSOR_P].c = ' ';
            CursorPosition(CURSOR_P);
        }
    }
    else if (key == KEY_UP) {
        ClearCurrentLine();
        if (currentHistoryIndex > 0) {
            currentHistoryIndex--;
            strcpy(commandBuffer, commandHistory[currentHistoryIndex]);
            commandLength = strlen(commandBuffer);

            CursorMove(CURSOR_P / MAX_COLUMNS, 0);
            ScreenDisplay(commandBuffer, BRIGHT_WHITE_COLOR);
            CURSOR_P += commandLength;
        }
    }
    else if (key == KEY_DOWN) {
        ClearCurrentLine();
        if (currentHistoryIndex < historyCount - 1) {
            currentHistoryIndex++;
            strcpy(commandBuffer, commandHistory[currentHistoryIndex]);
        }
        else {
            currentHistoryIndex = historyCount;
            commandBuffer[0] = '\0';
        }
        commandLength = strlen(commandBuffer);

        CursorMove(CURSOR_P / MAX_COLUMNS, 0);
        ScreenDisplay(commandBuffer, BRIGHT_WHITE_COLOR);
        CURSOR_P += commandLength;
    }
    else if (commandLength < MAX_COMMAND_LENGTH - 1) {
        commandBuffer[commandLength++] = key;
        char tempBuffer[2] = { key, '\0' };
        ScreenDisplay(tempBuffer, BRIGHT_WHITE_COLOR);
    }
}
