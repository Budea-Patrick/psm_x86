#ifndef _PIC_H_
#define _PIC_H_

#include <intrin.h>

typedef unsigned __int8     BYTE, * PBYTE;

#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_COMMAND 0xA0
#define PIC_SLAVE_DATA 0xA1

#define ICW1_INIT 0x10 // init
#define ICW1_ICW4 0x01 // ICW4 present
#define ICW4_8086 0x01 // 8086 mode

#define PIC_MASTER_REMAP_OFFSET 0x20
#define PIC_SLAVE_REMAP_OFFSET 0x28
#define PIC_EOI 0x20

void PICRemap(void);
void SendEOI(BYTE irq);
void PICMaskIrqs(void);

#endif