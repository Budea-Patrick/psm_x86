#ifndef PS2_H
#define PS2_H

#include <intrin.h>

typedef unsigned __int8 BYTE;

// PS/2 Controller ports
#define PS2_COMMAND_PORT 0x64
#define PS2_DATA_PORT    0x60

// PS/2 Controller commands
#define PS2_CMD_DISABLE_FIRST_PORT  0xAD
#define PS2_CMD_ENABLE_FIRST_PORT   0xAE  // Command to enable the first PS/2 port
#define PS2_CMD_READ_CONFIG         0x20
#define PS2_CMD_WRITE_CONFIG        0x60
#define PS2_CMD_SELF_TEST           0xAA
#define PS2_CMD_TEST_FIRST_PORT     0xAB  // Command to test the first PS/2 port
#define PS2_CMD_RESET_DEVICE        0xFF

// Self-test and port-test responses
#define PS2_SELF_TEST_PASS          0x55
#define PS2_PORT_TEST_PASS          0x00  // Expected response if port test passes

// Bit masks for configuration modifications
#define PS2_CONFIG_ENABLE_IRQ1         0x01  // Set bit 0 to enable IRQ for port 1
#define PS2_CONFIG_DISABLE_IRQ1        0xFE  // Clear bit 0 to disable IRQ for port 1
#define PS2_CONFIG_DISABLE_TRANSLATION 0xBF  // Clear bit 6 to disable translation for port 1
#define PS2_CONFIG_ENABLE_CLOCK1       0xEF  // Clear bit 4 to enable the clock for port 1

// Device reset responses
#define PS2_RESET_ACK               0xFA  // Acknowledgment for reset command
#define PS2_RESET_SUCCESS           0xAA  // Success code after reset
#define PS2_RESET_FAIL              0xFC  // Failure code for device reset

// Function prototypes
void PS2_SendCommand(BYTE command);
void PS2_DisableKeyboardPort();
void PS2_FlushOutputBuffer();
BYTE PS2_ReadConfigByte();
void PS2_WriteConfigByte(BYTE config);
void PS2_ConfigureKeyboard();
BYTE PS2_SelfTest();
BYTE PS2_TestKeyboardPort();
void PS2_EnableKeyboardPort();
BYTE PS2_ResetKeyboard();
void PS2_SetScancodeSet1();

#endif // PS2_H
