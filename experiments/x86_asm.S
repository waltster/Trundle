.text

.global _main

_main:
    movq $0x2000004, %rax
    movq $1, %rdi
    leaq my_string(%rip), %rsi
    movq $4, %rdx
    syscall

    ret

.data
my_string:
    .asciz "abc\n"
