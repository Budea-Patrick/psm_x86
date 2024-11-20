#include "main.h"
#include "string.h"
#include "screen.h"
#include "logging.h"
#include "pic.h"
#include "pit.h"
#include "ps2.h"
#include "scancodes.h"
#include "ata.h"

#define SECTOR_SIZE 512

IDT_ENTRY idt[256];
IDTR idtr;

void load_idt() {
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (QWORD)&idt;

    __lidt(&idtr);
}

void set_idt_entry(int index, void (*handler)()) {
    QWORD handler_address = (QWORD)handler;

    idt[index].LowWordOffset = handler_address & 0xFFFF;
    idt[index].SegmentSelector = 0x08;
    idt[index].IST = 0;
    idt[index].Type = 0xE;
    idt[index].DPL = 0;
    idt[index].Present = 1;
    idt[index].HighWordOffset = (handler_address >> 16) & 0xFFFF;
    idt[index].HighestDwordOffset = (handler_address >> 32);
    idt[index].Reserved0 = 0;
    idt[index].Reserved1 = 0;
    idt[index].Reserved = 0;
}

void TriggerPageFault() {
    Log("Enetered page fault function\n");
    volatile int* ptr = (int*)0xDEADBEEF;
    *ptr = 50;
}

#pragma optimize("", off)
void TriggerDivideError() {
    Log("Entered divide fault function\n");
    volatile int x = 0;
    int y = 10 / x;
}
#pragma optimize("", on)

void InterruptCommonHandle(
    BYTE InterruptIndex,                     // [0x0, 0xFF]
    INTERRUPT_STACK_COMPLETE* StackPointer,
    BYTE ErrorCodeAvailable,                 // 0 if not available
    COMPLETE_PROCESSOR_STATE* ProcessorState // Pointer to a structure containing CPU register state
) {
    //LogSerial(0, "This is the interrupt index that has been found: %X\n", InterruptIndex);

    /*Log("Registers:\n--------------\n");
    LogSerial(0, "rax: 0x%X, rbx: 0x%X, rcx: 0x%X\n", ProcessorState->rax, ProcessorState->rbx, ProcessorState->rcx);
    LogSerial(0, "rdx: 0x%X, rsi: 0x%X, rdi: 0x%X\n", ProcessorState->rdx, ProcessorState->rsi, ProcessorState->rdi);
    LogSerial(0, "rip: 0x%X, rsp: 0x%X, rbp: 0x%X\n", StackPointer->rip, StackPointer->rsp, ProcessorState->rbp);
    LogSerial(0, "r8: 0x%X, r9: 0x%X, r10: 0x%X\n", ProcessorState->r8, ProcessorState->r9, ProcessorState->r10);
    LogSerial(0, "r11: 0x%X, r12: 0x%X, r13: 0x%X\n", ProcessorState->r11, ProcessorState->r12, ProcessorState->r13);
    LogSerial(0, "r14: 0x%X, r15: 0x%X\n", ProcessorState->r14, ProcessorState->r15);
    LogSerial(0, "cs: 0x%X, ss: 0x%X, ds: 0x%X, ", StackPointer->cs, StackPointer->ss, ProcessorState->ds);
    LogSerial(0, "es: 0x%X, fs: 0x%X, gs: 0x%X, ", ProcessorState->es, ProcessorState->fs, ProcessorState->gs);
    LogSerial(0, "efl: 0x%X\n", StackPointer->rflags);

    Log("Stack:\n--------------\n");
    QWORD* printing_stack = (QWORD*)StackPointer->rsp;
    for (QWORD i = 0; i < 16; i++) {
        LogSerial(0, "%X %X\n", printing_stack + i, *(printing_stack + i));
    }*/

    static int extended = 0;
    if (InterruptIndex == 32) {
        ticks++;
        if (ticks < 150) {
            //LogSerial(0, "Timer tick: %D\n", ticks);
        }
    }
    if (InterruptIndex == 33) {
        BYTE scancode = __inbyte(PS2_DATA_PORT);
        if (scancode == 0xE0) {
            extended = 1;
        }
        else {
            if (scancode & 0x80) {
            }
            else {
                DisplayKeyPress(scancode, extended);
            }
            extended = 0;
        }
    }

    // came from pic
    if (InterruptIndex > 31 && InterruptIndex < 48) {
        SendEOI(InterruptIndex - 32);
    }
}

void PICSetup() {
    PICRemap();
    PICMaskIrqs();
}

void PS2Setup() {

    PS2_DisableKeyboardPort();
    PS2_FlushOutputBuffer();
    PS2_ConfigureKeyboard();
    if (!PS2_SelfTest()) {
        Log("PS2 setup has failed\n");
        return;
    }

    if (!PS2_TestKeyboardPort()) {
        Log("PS2 setup has failed\n");
        return;
    }
    PS2_EnableKeyboardPort();
    BYTE reset = PS2_ResetKeyboard();
    if (reset != 1) {
        Log("PS2 setup has failed\n");
        return;
    }

    PS2_SetScancodeSet1();
    Log("Keyboard controller has been set up\n");
}


void KernelMain()
{
    ticks = 0;
    //__magic();    // break into BOCHS

    __enableSSE();  // only for demo; in the future will be called from __init.asm

    ClearScreen(NULL, 0, NULL);

    InitLogging();

    Log("Logging initialized!\n");
    Log("Setting up the idt entries\n");

    set_idt_entry(0, _isr_stub_0);
    set_idt_entry(1, _isr_stub_1);
    set_idt_entry(2, _isr_stub_2);
    set_idt_entry(3, _isr_stub_3);
    set_idt_entry(4, _isr_stub_4);
    set_idt_entry(5, _isr_stub_5);
    set_idt_entry(6, _isr_stub_6);
    set_idt_entry(7, _isr_stub_7);
    set_idt_entry(8, _isr_stub_8);
    set_idt_entry(9, _isr_stub_9);
    set_idt_entry(10, _isr_stub_10);
    set_idt_entry(11, _isr_stub_11);
    set_idt_entry(12, _isr_stub_12);
    set_idt_entry(13, _isr_stub_13);
    set_idt_entry(14, _isr_stub_14);
    set_idt_entry(16, _isr_stub_16);
    set_idt_entry(17, _isr_stub_17);
    set_idt_entry(18, _isr_stub_18);
    set_idt_entry(19, _isr_stub_19);
    set_idt_entry(20, _isr_stub_20);
    set_idt_entry(32, _isr_stub_32); // IDT entry for PIT
    set_idt_entry(33, _isr_stub_33); // IDT entry for PS2

    Log("idt entries have been set up, loading the idt using inline assembly\n");

    load_idt();
    Log("Loaded the IDT\n");

    // 0 div
    /*Log("Triggering a divide error\n");
    __magic();
    TriggerDivideError();*/

    // page fault
    /*Log("Triggering a page fault exception\n");
    TriggerPageFault();*/

    Log("Setting up the PIC\n");
    PICSetup();
    Log("PIC has been set up\n");

    Log("Setting up the PIT\n");
    PITSetup(20);
    Log("PIT has been set up\n");

    Log("Setting up the keyboard\n");
    PS2Setup();

    Log("Searching ATA devices\n");
    int found_bus = -1, found_device = -1;

    /*for (int bus = 0; bus < 2; bus++) {
        for (int device = 0; device < 2; device++) {
            LogSerial(0, "Checking bus %D, device %D...\n", bus, device);
            int result = ata_identify_device(bus, device);
            if (result == 0) {
                LogSerial(0, "Device found on bus %D, device %D\n", bus, device);
            }
            else if (result == -1) {
                LogSerial(0, "Error detecting device on bus %D, device %D\n", bus, device);
            }
            else {
                LogSerial(0, "No device found on bus %D, device %D\n", bus, device);
            }
        }
    }*/

    for (int bus = 0; bus < 2; bus++) {
        for (int device = 0; device < 2; device++) {
            LogSerial(0, "Checking bus %D, device %D...\n", bus, device);
            int result = ata_identify_device(bus, device);
            if (result == 0) {
                LogSerial(0, "Device found on bus %D, device %D\n", bus, device);
                found_bus = bus;
                found_device = device;
            }
        }
    }

    if (found_bus == -1 || found_device == -1) {
        Log("No ATA device found! Halting...\n");
    }

    WORD data[SECTOR_SIZE / 2];
    for (int i = 0; i < 256; i++) {
        data[i] = 0xABCD;
    }

    DWORD lba = 0;
    WORD sector_count = 1;

    /*int write_result = ata_write_sectors(found_bus, found_device, lba, sector_count, data);
    if (write_result == 0) {
        Log("ATA write operation successful!\n");
    }
    else {
        Log("ATA write operation failed!\n");
    }*/

    WORD read_buffer[SECTOR_SIZE / 2];
    Log("Reading first sector from the disk...\n");
    ata_read_sectors(found_bus, found_device, lba, sector_count, read_buffer);
    for (int i = 0; i < SECTOR_SIZE / 16; i++) {
        LogSerial(0, "%08X: ", i * 16);

        for (int j = 0; j < 8; j++) {
            WORD word = read_buffer[i * 8 + j];
            BYTE high_byte = (word >> 8) & 0xFF;
            BYTE low_byte = word & 0xFF;
            LogSerial(0, "%02X %02X ", high_byte, low_byte);
        }


        LogSerial(0, " | ");
        for (int j = 0; j < 8; j++) {
            WORD raw_word = read_buffer[i * 8 + j];
            BYTE high_byte = (raw_word >> 8) & 0xFF;
            BYTE low_byte = raw_word & 0xFF;

            LogSerial(0, "%c", high_byte);
            LogSerial(0, "%c", low_byte);
        }

        LogSerial(0, "\n");
    }

    __sti();

    while (1) {
        __halt();
    }

    // TODO!!! PIC programming; see http://www.osdever.net/tutorials/view/programming-the-pic
    // 
    // TODO!!! define interrupt routines and dump trap frame

    // TODO!!! Timer programming

    // TODO!!! Keyboard programming

    // TODO!!! Implement a simple console

    // TODO!!! read disk sectors using PIO mode ATA

    // TODO!!! Memory management: virtual, physical and heap memory allocators
}
