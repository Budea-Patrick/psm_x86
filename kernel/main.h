#ifndef _MAIN_H_
#define _MAIN_H_

#include <intrin.h>

//
// default types
//
typedef unsigned __int8     BYTE, * PBYTE;
typedef unsigned __int16    WORD, * PWORD;
typedef unsigned __int32    DWORD, * PDWORD;
typedef unsigned __int64    QWORD, * PQWORD;
typedef signed __int8       INT8;
typedef signed __int16      INT16;
typedef signed __int32      INT32;
typedef signed __int64      INT64;

volatile QWORD ticks;

//
// exported functions from __init.asm
//
void __cli(void);
void __sti(void);
void __magic(void);         // MAGIC breakpoint into BOCHS (XCHG BX,BX)
void __enableSSE(void);

extern void _isr_stub_0();
extern void _isr_stub_1();
extern void _isr_stub_2();
extern void _isr_stub_3();
extern void _isr_stub_4();
extern void _isr_stub_5();
extern void _isr_stub_6();
extern void _isr_stub_7();
extern void _isr_stub_8();
extern void _isr_stub_9();
extern void _isr_stub_10();
extern void _isr_stub_11();
extern void _isr_stub_12();
extern void _isr_stub_13();
extern void _isr_stub_14();
extern void _isr_stub_16();
extern void _isr_stub_17();
extern void _isr_stub_18();
extern void _isr_stub_19();
extern void _isr_stub_20();

extern void _isr_stub_32();
extern void _isr_stub_33(); // keyboard

#pragma pack(push, 1)
typedef struct _IDT_ENTRY
{
    // 15:0
    WORD            LowWordOffset;              // Bits 15:0 of address

    // 31:16
    WORD            SegmentSelector;

    // 34:32
    // IST = Interrupt Stack Table
    // if set to 0 will use legacy stack switching
    // else will use IST entry from TSS
    WORD            IST : 3;
    WORD            Reserved0 : 5;    // these must be 0 on x64
    WORD            Type : 4;    // you want interrupt gates, see Table 3-2 System-Segment and Gate-Descriptor Types
    WORD            Reserved1 : 1;    // 0
    WORD            DPL : 2;
    WORD            Present : 1;
    WORD            HighWordOffset;     // Bits 31:16 of address
    DWORD           HighestDwordOffset; // Bits 63:32 of address
    DWORD           Reserved;
} IDT_ENTRY;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _IDTR
{
    WORD limit;
    QWORD base;
} IDTR;
#pragma pack(pop)

typedef struct _COMPLETE_PROCESSOR_STATE {
    QWORD rax;
    QWORD rbx;
    QWORD rcx;
    QWORD rdx;
    QWORD rsi;
    QWORD rdi;
    QWORD rbp;
    QWORD r8;
    QWORD r9;
    QWORD r10;
    QWORD r11;
    QWORD r12;
    QWORD r13;
    QWORD r14;
    QWORD r15;

    WORD ds;
    WORD es;
    WORD fs;
    WORD gs;
} COMPLETE_PROCESSOR_STATE;

#pragma pack(push, 1)
typedef struct _INTERRUPT_STACK_COMPLETE {
    QWORD ErrorCode;
    QWORD rip;
    QWORD cs;
    QWORD rflags;
    QWORD rsp;
    QWORD ss;
} INTERRUPT_STACK_COMPLETE;
#pragma pack(pop)

void
InterruptCommonHandle(
    BYTE                     InterruptIndex,
    INTERRUPT_STACK_COMPLETE* StackPointer,
    BYTE                     ErrorCodeAvailable,
    COMPLETE_PROCESSOR_STATE* ProcessorState
);

void PICSetup(void);
void PS2Setup(void);

#endif // _MAIN_H_