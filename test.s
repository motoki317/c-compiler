.intel_syntax noprefix
.global plus, main

plus:
        add rsi, rdi
        mov rax, rsi
        ret

main:
        mov rdi, 5
        mov rsi, 4
        call plus
        ret
