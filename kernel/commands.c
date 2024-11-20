#include "commands.h"
#include "main.h"
#include "screen.h"
#include "ata.h"

#define SECTOR_SIZE 512

void PrintMBR() {
    int found_bus = -1, found_device = -1;
    for (int bus = 0; bus < 2; bus++) {
        for (int device = 0; device < 2; device++) {
            int result = ata_identify_device(bus, device);
            if (result == 0) {
                found_bus = bus;
                found_device = device;
            }
        }
    }

    if (found_bus == -1 || found_device == -1) {
        ScreenDisplay("No ATA device\n", BRIGHT_RED_COLOR);
        return;
    }

    WORD read_buffer[SECTOR_SIZE / 2];
    DWORD lba = 0;
    WORD sector_count = 1;
    ata_read_sectors(found_bus, found_device, lba, sector_count, read_buffer);

    char lineBuffer[128];
    for (int i = 0; i < SECTOR_SIZE / 16; i++) {
        DWORD offset = i * 16;

        cl_snprintf(lineBuffer, sizeof(lineBuffer), "%08X: ", offset);
        ScreenDisplay(lineBuffer, BRIGHT_WHITE_COLOR);

        for (int j = 0; j < 8; j++) {
            WORD raw_word = read_buffer[i * 8 + j];
            BYTE high_byte = (raw_word >> 8) & 0xFF;
            BYTE low_byte = raw_word & 0xFF;
            cl_snprintf(lineBuffer, sizeof(lineBuffer), "%02X %02X ", high_byte, low_byte);
            ScreenDisplay(lineBuffer, BRIGHT_WHITE_COLOR);
        }

        ScreenDisplay(" | ", BRIGHT_WHITE_COLOR);
        for (int j = 0; j < 8; j++) {
            WORD word = read_buffer[i * 8 + j];
            BYTE high_byte = (word >> 8) & 0xFF;
            BYTE low_byte = word & 0xFF;

            char high_ascii[2] = { high_byte, '\0' };
            char low_ascii[2] = { low_byte, '\0' };
            ScreenDisplay(high_ascii, BRIGHT_WHITE_COLOR);
            ScreenDisplay(low_ascii, BRIGHT_WHITE_COLOR);
        }

        MoveCursorToNextLine();
    }
}

void CLIProcessCommand(char* command) {
    if (cl_strcmp(command, "clear") == 0 || cl_strcmp(command, "cls") == 0) {
        ClearScreen(NULL, 0, NULL);
    }
    else if (cl_strcmp(command, "time") == 0) {
        MoveCursorToNextLine();
        char tickBuffer[20];
        itoa(&ticks, FALSE, tickBuffer, 10, TRUE);
        ScreenDisplay(tickBuffer, BRIGHT_WHITE_COLOR);
        MoveCursorToNextLine();
        BYTE* dateTime = GetCurrentDate();
        ScreenDisplay(dateTime, BRIGHT_WHITE_COLOR);
        MoveCursorToNextLine();
    }
    else if (cl_strcmp(command, "edit") == 0) {
        EnterEditMode();
    }
    else if (cl_strcmp(command, "printmbr") == 0) {
        PrintMBR();
    }
    else {
        MoveCursorToNextLine();
        ScreenDisplay("Unknown command", BRIGHT_WHITE_COLOR);
        MoveCursorToNextLine();
    }
}

