; =========================================
; MZV-2025 Compiler - Generated Assembly
; Target: Windows x64 (MASM)
; =========================================

.686
.model flat, stdcall
option casemap:none

; External functions from C runtime
includelib kernel32.lib
includelib msvcrt.lib

ExitProcess PROTO :DWORD
printf PROTO C :VARARG
SetConsoleOutputCP PROTO :DWORD

.data

    fmt_int    db "%d", 10, 0        ; Формат для integer
    fmt_char   db "%c", 10, 0        ; Формат для char
    fmt_str    db "%s", 10, 0        ; Формат для string

    ; String literals
    str_0 db "Start", 0
    str_1 db "big", 0
    str_2 db "medium", 0
    str_3 db "small", 0
    str_4 db "ok", 0
    str_5 db "End", 0

    ; Глобальные переменные

.code


    ; Функция: abs
abs PROC
    push ebp
    mov ebp, esp
    sub esp, 64
    push ebx
    push esi
    push edi
    mov eax, DWORD PTR [ebp+8]
    mov DWORD PTR [ebp-8], eax

    ; if
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, 0
    push eax
    pop ebx
    pop eax
    cmp eax, ebx
    jl cmp_true_2
    mov eax, 0
    jmp cmp_end_3
cmp_true_2:
    mov eax, 1
cmp_end_3:
    cmp eax, 0
    je else_0
    mov eax, 0
    push eax
    mov eax, DWORD PTR [ebp-8]
    push eax
    pop ebx
    pop eax
    sub eax, ebx
    mov DWORD PTR [ebp-16], eax
    jmp endif_1
else_0:
    mov eax, DWORD PTR [ebp-8]
    mov DWORD PTR [ebp-16], eax
endif_1:

    ; return
    mov eax, DWORD PTR [ebp-16]
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
abs ENDP

    ; Функция: max
max PROC
    push ebp
    mov ebp, esp
    sub esp, 80
    push ebx
    push esi
    push edi
    mov eax, DWORD PTR [ebp+8]
    mov DWORD PTR [ebp-8], eax
    mov eax, DWORD PTR [ebp+12]
    mov DWORD PTR [ebp-16], eax

    ; if
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, DWORD PTR [ebp-16]
    push eax
    pop ebx
    pop eax
    cmp eax, ebx
    jg cmp_true_6
    mov eax, 0
    jmp cmp_end_7
cmp_true_6:
    mov eax, 1
cmp_end_7:
    cmp eax, 0
    je else_4
    mov eax, DWORD PTR [ebp-8]
    mov DWORD PTR [ebp-24], eax
    jmp endif_5
else_4:
    mov eax, DWORD PTR [ebp-16]
    mov DWORD PTR [ebp-24], eax
endif_5:

    ; return
    mov eax, DWORD PTR [ebp-24]
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
max ENDP

    ; Функция: calc
calc PROC
    push ebp
    mov ebp, esp
    sub esp, 80
    push ebx
    push esi
    push edi
    mov eax, DWORD PTR [ebp+8]
    mov DWORD PTR [ebp-8], eax
    mov eax, DWORD PTR [ebp+12]
    mov DWORD PTR [ebp-16], eax
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, DWORD PTR [ebp-16]
    push eax
    pop ebx
    pop eax
    imul eax, ebx
    push eax
    mov eax, DWORD PTR [ebp-8]
    push eax
    pop ebx
    pop eax
    add eax, ebx
    push eax
    mov eax, DWORD PTR [ebp-16]
    push eax
    pop ebx
    pop eax
    sub eax, ebx
    mov DWORD PTR [ebp-24], eax

    ; return
    mov eax, DWORD PTR [ebp-24]
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
calc ENDP

    ; Главная функция
main PROC
    push ebp
    mov ebp, esp
    sub esp, 112
    push ebx
    push esi
    push edi

    ; Установка кодовой страницы CP1251 для кириллицы
    push 1251
    call SetConsoleOutputCP
    mov eax, 25
    mov DWORD PTR [ebp-8], eax
    mov eax, 10
    mov DWORD PTR [ebp-16], eax
    mov eax, 88
    mov DWORD PTR [ebp-40], eax

    ; output
    mov eax, OFFSET str_0
    push eax
    push OFFSET fmt_str
    call printf
    add esp, 8

    ; output
    mov eax, DWORD PTR [ebp-8]
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8

    ; output
    mov eax, DWORD PTR [ebp-16]
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, DWORD PTR [ebp-16]
    push eax
    mov eax, 2
    push eax
    pop ebx
    pop eax
    imul eax, ebx
    push eax
    pop ebx
    pop eax
    add eax, ebx
    mov DWORD PTR [ebp-24], eax

    ; output
    mov eax, DWORD PTR [ebp-24]
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8

    ; Унарная операция
    mov eax, DWORD PTR [ebp-8]
    inc eax
    mov DWORD PTR [ebp-8], eax

    ; Унарная операция
    mov eax, DWORD PTR [ebp-16]
    dec eax
    mov DWORD PTR [ebp-16], eax
    mov eax, DWORD PTR [ebp-8]
    not eax
    mov DWORD PTR [ebp-24], eax

    ; if
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, DWORD PTR [ebp-16]
    push eax
    pop ebx
    pop eax
    cmp eax, ebx
    jg cmp_true_10
    mov eax, 0
    jmp cmp_end_11
cmp_true_10:
    mov eax, 1
cmp_end_11:
    cmp eax, 0
    je else_8

    ; if
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, 50
    push eax
    pop ebx
    pop eax
    cmp eax, ebx
    jg cmp_true_14
    mov eax, 0
    jmp cmp_end_15
cmp_true_14:
    mov eax, 1
cmp_end_15:
    cmp eax, 0
    je else_12

    ; output
    mov eax, OFFSET str_1
    push eax
    push OFFSET fmt_str
    call printf
    add esp, 8
    jmp endif_13
else_12:

    ; output
    mov eax, OFFSET str_2
    push eax
    push OFFSET fmt_str
    call printf
    add esp, 8
endif_13:
    jmp endif_9
else_8:

    ; output
    mov eax, OFFSET str_3
    push eax
    push OFFSET fmt_str
    call printf
    add esp, 8
endif_9:
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, DWORD PTR [ebp-16]
    push eax
    pop DWORD PTR [ebp-200]
    pop DWORD PTR [ebp-204]
    push DWORD PTR [ebp-200]
    push DWORD PTR [ebp-204]
    ; Вызов max
    call max
    add esp, 8
    mov DWORD PTR [ebp-24], eax

    ; output
    mov eax, DWORD PTR [ebp-24]
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8
    mov eax, 0
    push eax
    mov eax, 15
    push eax
    pop ebx
    pop eax
    sub eax, ebx
    mov DWORD PTR [ebp-32], eax
    mov eax, DWORD PTR [ebp-32]
    push eax
    ; Вызов abs
    call abs
    add esp, 4
    mov DWORD PTR [ebp-24], eax

    ; output
    mov eax, DWORD PTR [ebp-24]
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8
    mov eax, 5
    push eax
    mov eax, 3
    push eax
    pop DWORD PTR [ebp-200]
    pop DWORD PTR [ebp-204]
    push DWORD PTR [ebp-200]
    push DWORD PTR [ebp-204]
    ; Вызов calc
    call calc
    add esp, 8
    mov DWORD PTR [ebp-24], eax

    ; output
    mov eax, DWORD PTR [ebp-24]
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8

    ; if
    mov eax, DWORD PTR [ebp-24]
    push eax
    mov eax, 10
    push eax
    pop ebx
    pop eax
    cmp eax, ebx
    jg cmp_true_18
    mov eax, 0
    jmp cmp_end_19
cmp_true_18:
    mov eax, 1
cmp_end_19:
    cmp eax, 0
    je else_16

    ; output
    mov eax, OFFSET str_4
    push eax
    push OFFSET fmt_str
    call printf
    add esp, 8
    jmp endif_17
else_16:
endif_17:

    ; output
    mov eax, DWORD PTR [ebp-40]
    push eax
    push OFFSET fmt_char
    call printf
    add esp, 8

    ; output
    mov eax, OFFSET str_5
    push eax
    push OFFSET fmt_str
    call printf
    add esp, 8

    ; return
    mov eax, 0
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
main ENDP

end main
