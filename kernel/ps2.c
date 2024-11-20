#include "ps2.h"
#include "main.h"
#include "logging.h"

void PS2_SendCommand(BYTE command) {
    // clear before write
    while (__inbyte(PS2_COMMAND_PORT) & 0x02);
    __outbyte(PS2_COMMAND_PORT, command);
}

void PS2_DisableKeyboardPort() {
    PS2_SendCommand(PS2_CMD_DISABLE_FIRST_PORT);
}

void PS2_FlushOutputBuffer() {
    // check for data
    while ((__inbyte(PS2_COMMAND_PORT) & 0x01)) {
        __inbyte(PS2_DATA_PORT);
    }
}

BYTE PS2_ReadConfigByte() {
    PS2_SendCommand(PS2_CMD_READ_CONFIG);
    while (!(__inbyte(PS2_COMMAND_PORT) & 0x01));
    return __inbyte(PS2_DATA_PORT);
}

void PS2_WriteConfigByte(BYTE config) {
    PS2_SendCommand(PS2_CMD_WRITE_CONFIG);
    while (__inbyte(PS2_COMMAND_PORT) & 0x02);
    __outbyte(PS2_DATA_PORT, config);
}

void PS2_ConfigureKeyboard() {
    BYTE config = PS2_ReadConfigByte();
    config = config & PS2_CONFIG_DISABLE_IRQ1;          // 0xFE
    config = config & PS2_CONFIG_DISABLE_TRANSLATION;   // 0xBF
    config = config & PS2_CONFIG_ENABLE_CLOCK1;         // 0xEF
    PS2_WriteConfigByte(config);
}

BYTE PS2_SelfTest() {
    BYTE originalConfig = PS2_ReadConfigByte();

    PS2_SendCommand(PS2_CMD_SELF_TEST);
    while (!(__inbyte(PS2_COMMAND_PORT) & 0x01));
    BYTE response = __inbyte(PS2_DATA_PORT);
    if (response == PS2_SELF_TEST_PASS) {
        return 1;
    }
    else {
        PS2_WriteConfigByte(originalConfig);
        return 0;
    }
}

BYTE PS2_TestKeyboardPort() {
    PS2_SendCommand(PS2_CMD_TEST_FIRST_PORT);
    while (!(__inbyte(PS2_COMMAND_PORT) & 0x01));
    BYTE result = __inbyte(PS2_DATA_PORT);

    return (result == PS2_PORT_TEST_PASS) ? 1 : 0;
}

void PS2_EnableKeyboardPort() {
    PS2_SendCommand(PS2_CMD_ENABLE_FIRST_PORT);

    BYTE config = PS2_ReadConfigByte();
    config = config | PS2_CONFIG_ENABLE_IRQ1;
    PS2_WriteConfigByte(config);
}

BYTE PS2_ResetKeyboard() {
    while (__inbyte(PS2_COMMAND_PORT) & 0x02);
    __outbyte(PS2_DATA_PORT, PS2_CMD_RESET_DEVICE);

    // response - 0xFA, 0xAA, device PS/2 ID
    // first response
    while (!(__inbyte(PS2_COMMAND_PORT) & 0x01));
    BYTE response = __inbyte(PS2_DATA_PORT);
    if (response != PS2_RESET_ACK) {
        return 0; // reset failed or no device present
    }

    // second response
    while (!(__inbyte(PS2_COMMAND_PORT) & 0x01));
    response = __inbyte(PS2_DATA_PORT);
    if (response == PS2_RESET_SUCCESS) {
        return 1; // successful
    }
    else if (response == PS2_RESET_FAIL) {
        return -1; // failed
    }

    // no response, port is empty
    return 0;
}

void PS2_SetScancodeSet1() {
    while (__inbyte(PS2_COMMAND_PORT) & 0x02);
    __outbyte(PS2_DATA_PORT, 0xF0);

    while (__inbyte(PS2_COMMAND_PORT) & 0x02);
    __outbyte(PS2_DATA_PORT, 0x01);

    while (!(__inbyte(PS2_COMMAND_PORT) & 0x01));
    BYTE response = __inbyte(PS2_DATA_PORT);
    if (response != 0xFA) {
        Log("Failed to set Scancode Set 1\n");
    }
    else {
        Log("Scancode Set 1 enabled\n");
    }
}