#include "pic.h"

void PICRemap(void) {
    BYTE a1, a2;

    // save current interrupt masks
    a1 = __inbyte(PIC_MASTER_DATA);
    a2 = __inbyte(PIC_SLAVE_DATA);

    // init sequence for PICs
    __outbyte(PIC_MASTER_COMMAND, ICW1_INIT | ICW1_ICW4);
    __outbyte(PIC_SLAVE_COMMAND, ICW1_INIT | ICW1_ICW4);

    // remapping offset
    __outbyte(PIC_MASTER_DATA, PIC_MASTER_REMAP_OFFSET);
    __outbyte(PIC_SLAVE_DATA, PIC_SLAVE_REMAP_OFFSET);

    __outbyte(PIC_MASTER_DATA, 4); // master has slave at irq2
    __outbyte(PIC_SLAVE_DATA, 2);  // slave PIC cascade

    // use 8086 mode
    __outbyte(PIC_MASTER_DATA, ICW4_8086);
    __outbyte(PIC_SLAVE_DATA, ICW4_8086);

    // restore masks
    __outbyte(PIC_MASTER_DATA, a1);
    __outbyte(PIC_SLAVE_DATA, a2);
}


void SendEOI(BYTE irq) {
    if (irq > 7) {
        __outbyte(PIC_SLAVE_COMMAND, PIC_EOI);
    }
    __outbyte(PIC_MASTER_COMMAND, PIC_EOI);
}

void PICMaskIrqs(void) {
    __outbyte(PIC_MASTER_DATA, 0xFC); // 1111 1100 -> irq2 - irq7
    __outbyte(PIC_SLAVE_DATA, 0xFF); // 1111 1111 ->  irq8 - irq15
}
