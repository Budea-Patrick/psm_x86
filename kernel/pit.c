#include "pit.h"

void PITSetup(DWORD frequency) {
	DWORD div = PIT_FREQ / frequency;
	__outbyte(PIT_COMMAND, PIT_MODE_3); // square wave generation
	__outbyte(PIT_CHANNEL_0, (BYTE)(div & 0xFF));
	__outbyte(PIT_CHANNEL_0, (BYTE)((div >> 8) & 0xFF));
}
