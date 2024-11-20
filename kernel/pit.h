#ifndef _PIT_H_
#define _PIT_H_

#include <intrin.h>

typedef unsigned __int32    DWORD, * PDWORD;
typedef unsigned __int8     BYTE, * PBYTE;

#define PIT_CHANNEL_0        0x40  // data port
#define PIT_COMMAND          0x43  // command port

#define PIT_FREQ   1193182
#define PIT_MODE_3 0x36

void PITSetup(DWORD frequency);

#endif