.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    movl 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    jmp $0x08, $.flush
.flush:
    ret

.global idt_flush
.type idt_flush, @function
idt_flush:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

.macro isr_stub_no_err num
.global isr_\num
isr_\num:
    cli
    push $\num
    jmp isr_common_stub
.endm

.macro isr_stub_err num
.global isr_\num
isr_\num:
    cli
    push $0
    push $\num
    jmp isr_common_stub
.endm

.macro irq_stub irq_num int_num
.global irq_\irq_num
irq_\irq_num:
    cli
    push $0
    push $\int_num
    jmp irq_common_stub
.endm

.global isr_common_stub
.extern isr_handler
isr_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    push %esp
 
    call isr_handler

    pop %ebx
    pop %ebx
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    popa
    add $8, %esp
    sti
    iret

.global irq_common_stub
irq_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    push %esp

    call irq_handler

    pop %ebx
    pop %ebx
    mov %bx, %ds
    mov %bx, %es
    mov %bx, %fs
    mov %bx, %gs
    popa
    add $8, %esp
    sti
    iret


isr_stub_no_err 0   // Division error
isr_stub_no_err 1   // Debug 
isr_stub_no_err 2   // Non-maskable interrupt
isr_stub_no_err 3   // Breakpoint
isr_stub_no_err 4   // Overflow
isr_stub_no_err 5   // Bound range exceeded
isr_stub_no_err 6   // Invalid opcode
isr_stub_no_err 7   // Device-not-available
isr_stub_err    8   // Double fault
isr_stub_no_err 9   // Coprosessor segment overrun
isr_stub_err    10  // Invalid TSS
isr_stub_err    11  // Segment not present
isr_stub_err    12  // Stack-Segment fault
isr_stub_err    13  // General protection fault
isr_stub_err    14  // Page fault
isr_stub_no_err 15  // Reserved
isr_stub_no_err 16  // x87 floating-point exception
isr_stub_err    17  // Alignment error
isr_stub_no_err 18  // Machine check
isr_stub_no_err 19  // SIMD Floating-point exception
isr_stub_no_err 20  // Virtualization exception
isr_stub_err    21  // Control protection exception
isr_stub_no_err 22  // Reserved
isr_stub_no_err 23  // Reserved
isr_stub_no_err 24  // Reserved
isr_stub_no_err 25  // Reserved
isr_stub_no_err 26
isr_stub_no_err 27
isr_stub_no_err 28  // Hypervisor injection exception
isr_stub_err    29  // VMM Communications exception
isr_stub_err    30  // Security exception
isr_stub_no_err 31  // Reserved

irq_stub 0 32
irq_stub 1 33
irq_stub 2 34
irq_stub 3 35
irq_stub 4 36
irq_stub 5 37
irq_stub 6 38
irq_stub 7 39
irq_stub 8 40
irq_stub 9 41
irq_stub 10 42
irq_stub 11 43
irq_stub 12 44
irq_stub 13 45
irq_stub 14 46
irq_stub 15 47

