; interrupts.asm - define macros for ISRs with and without error codes

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

segment .text
[BITS 64]

IMPORTFROMC InterruptCommonHandle

global _exception_handler
    
_exception_handler:

    mov [proc_state.rax], rax
    mov [proc_state.rbx], rbx
    mov [proc_state.rcx], rcx
    mov [proc_state.rdx], rdx
    mov [proc_state.rsi], rsi
    mov [proc_state.rdi], rdi
    mov [proc_state.rbp], rbp
    mov [proc_state.r8],  r8
    mov [proc_state.r9],  r9
    mov [proc_state.r10], r10
    mov [proc_state.r11], r11
    mov [proc_state.r12], r12
    mov [proc_state.r13], r13
    mov [proc_state.r14], r14
    mov [proc_state.r15], r15
    mov [proc_state.ds], ds
    mov [proc_state.es], es
    mov [proc_state.fs], fs
    mov [proc_state.gs], gs
        
    pop rbx    ; InterruptIndex
    pop r10    ; ErrorCodeAvailable
    mov rax, rsp ; StackPointer
    
    mov r9, proc_state
    mov r8, r10
    mov rdx, rax
    mov rcx, rbx
    
    sub rsp, 32

    call InterruptCommonHandle

    add rsp, 40 ; value required to return the execution to the proper instruction using rip
    iretq

%macro isr_err_stub 1
global _isr_stub_%+%1
_isr_stub_%+%1:
    push DWORD 1          ; There is an error code
    push DWORD %1         ; Interrupt index
    jmp _exception_handler ; Jump to common exception handler
%endmacro

%macro isr_no_err_stub 1
global _isr_stub_%+%1
_isr_stub_%+%1:
    push DWORD 0          ; Push a dummy error code for consistency
    push DWORD 0          ; There is no error code
    push DWORD %1         ; Interrupt index
    jmp _exception_handler ; Jump to common exception handler
%endmacro

; Define ISRs without error codes
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_no_err_stub 9
isr_no_err_stub 16
isr_no_err_stub 17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20

isr_no_err_stub 32   ; IRQ0 PIT
isr_no_err_stub 33

; Define ISRs with error codes
isr_err_stub 8
isr_err_stub 10
isr_err_stub 11
isr_err_stub 12
isr_err_stub 13
isr_err_stub 14

proc_state:
	.rax:			dq 0
	.rbx:			dq 0
	.rcx:			dq 0
	.rdx:			dq 0
	.rsi:			dq 0
	.rdi:			dq 0
	.rbp:			dq 0
	.r8:			dq 0
	.r9:			dq 0
	.r10:			dq 0
	.r11:			dq 0
	.r12:			dq 0
	.r13:			dq 0
	.r14:			dq 0
	.r15:			dq 0
	.ds:			dq 0
	.es:			dq 0
	.fs:			dq 0
	.gs:			dq 0