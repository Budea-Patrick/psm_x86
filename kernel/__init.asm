;;-----------------_DEFINITIONS ONLY_-----------------------
;; IMPORT FUNCTIONS FROM C
%macro IMPORTFROMC 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    extern _%1
    %1 equ _%1
    %else
    extern %1
    %endif
%rotate 1 
%endrep
%endmacro

;; EXPORT TO C FUNCTIONS
%macro EXPORT2C 1-*
%rep  %0
    %ifidn __OUTPUT_FORMAT__, win32 ; win32 builds from Visual C decorate C names using _ 
    global _%1
    _%1 equ %1
    %else
    global %1
    %endif
%rotate 1 
%endrep
%endmacro

%define break xchg bx, bx

IMPORTFROMC KernelMain

TOP_OF_STACK                equ 0x200000
KERNEL_BASE_PHYSICAL        equ 0x200000
;;-----------------^DEFINITIONS ONLY^-----------------------

segment .text
[BITS 32]
ASMEntryPoint:
    cli
    MOV     DWORD [0x000B8000], 'O1S1'
%ifidn __OUTPUT_FORMAT__, win32
    MOV     DWORD [0x000B8004], '3121'                  ; 32 bit build marker
%else
    MOV     DWORD [0x000B8004], '6141'                  ; 64 bit build marker
%endif



    MOV     ESP, TOP_OF_STACK                           ; just below the kernel

    ;TODO!!! define page tables; see https://wiki.osdev.org ,Intel's manual, http://www.brokenthorn.com/Resources/ ,http://www.jamesmolloy.co.uk/tutorial_html/

    ;TODO!!! activate pagging
    
    ;TODO!!! transition to 64bits-long mode

    mov eax, cr4 ; save state of cr4
    or eax, 0x20 ; set bit 5 to 1
    mov cr4, eax ; restore state of cr4 with updated PAE flag


    align 0x1000
    PML4:
        dq 0

    align 0x1000
    PDPT:
        dq 0

    align 0x1000
    PDT:
        times 2 dq 0

    align 0x1000
    PT1:
        times 512 dq 0

    align 0x1000
    PT2:
        times 512 dq 0


    mov eax, PDPT ; load the address of PPDT, maybe use the label PDPT
    or eax, 0x03 ; modify flags for present and rw
    mov [PML4], eax ; make PML4 point to PPDT

    mov eax, PDT; load the address of PD, maybe use label PD
    or eax, 0x03 ; modify flags for present and rw
    mov [PDPT], eax ; maxe PPDT point to PD

    mov eax, PT1; load address of PT1
    or eax, 0x03 ; modify flags for present and rw
    mov [PDT], eax ; make PD point to PT1

    mov eax, PT2; load address of PT2
    or eax, 0x03 ; modify flags for present and rw
    mov [PDT + 8], eax ; make PDT + 8 point to PT2

    ; PT1 -> physical memory of 4 mb from 0x00000000 -> 0x001FFFFF
    mov eax, 0x00000000 ; start of physical memory
    ; use nasm preprocessor
    %assign i 0
    %rep 512 
	    or eax, 0x03 ; set present and rw flags
	    mov [PT1 + 8 * i], eax ;
	    add eax, 0x1000 ; go to next page
    %assign i i + 1
    %endrep

    ; PT2 -> physical memory of 4 mb from 0x00200000 -> 0x003FFFFF
    mov eax, 0x00200000 ; start of physical memory
    ; use nasm preprocessor
    %assign i 0
    %rep 512 
		or eax, 0x03 ; set present and rw flags
	    mov [PT2 + 8 * i], eax ;
	    add eax, 0x1000 ; go to next page
    %assign i i + 1
    %endrep

    mov eax, PML4
    mov cr3, eax
	

    mov ecx, 0xC0000080
    rdmsr

    or eax, 0x100
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
	
	lgdt [GDT_NEW]
	
	jmp        8:.bits64
	
.bits64:
    mov    ax, 16       ; index of FLAT_DESCRIPTOR_DATA32 entry     
    mov    gs, ax      
    mov    ss, ax      
    mov    fs, ax

	[bits 64]
    MOV     RAX, KernelMain     ; after 64bits transition is implemented the kernel must be compiled on x64
    CALL    RAX
    
    break
    CLI
    HLT

;;--------------------------------------------------------

__cli:
    CLI
    RET

__sti:
    STI
    RET

__magic:
    XCHG    BX,BX
    RET
    
__enableSSE:                ;; enable SSE instructions (CR4.OSFXSR = 1)  
    MOV     RAX, CR4
    OR      EAX, 0x00000200
    MOV     CR4, RAX
    RET
    
EXPORT2C ASMEntryPoint, __cli, __sti, __magic, __enableSSE

FLAT_DESCRIPTOR_CODE64  equ 0x00AF9A0000000000  ; Code: Execute/Read
FLAT_DESCRIPTOR_DATA64  equ 0x00CF920000000000  ; Data: Read/Write

GDT_NEW:
    .limit  dw  GDTTable_NEW.end - GDTTable_NEW - 1
    .base   dd  GDTTable_NEW
	
GDTTable_NEW:
    .null     dq 0                          ;  0
    .code64   dq FLAT_DESCRIPTOR_CODE64     ; 8  ; Add 64-bit Code descriptor
    .data64   dq FLAT_DESCRIPTOR_DATA64     ; 16  ; Add 64-bit Data descriptor
    .end:
