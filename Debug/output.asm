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

.data

    fmt_int    db "%d", 10, 0        ; Формат для integer
    fmt_char   db "%c", 10, 0        ; Формат для char
    fmt_str    db "%s", 10, 0        ; Формат для string

    ; Глобальные переменные

.code


    ; Главная функция
main PROC
    push ebp
    mov ebp, esp
    sub esp, 64
    push ebx
    push esi
    push edi
    mov eax, 10
    push eax
    pop eax
    mov DWORD PTR [ebp-8], eax

    ; if
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, 5
    push eax
    pop ebx
    pop eax
    cmp eax, ebx
    jg cmp_true_2
    mov eax, 0
    jmp cmp_end_3
cmp_true_2:
    mov eax, 1
cmp_end_3:
    push eax
    pop eax
    cmp eax, 0
    je else_0

    ; if
    mov eax, DWORD PTR [ebp-8]
    push eax
    mov eax, 8
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
    push eax
    pop eax
    cmp eax, 0
    je else_4

    ; output
    mov eax, 1
    push eax
    pop eax
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8
    jmp endif_5
else_4:

    ; output
    mov eax, 2
    push eax
    pop eax
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8
endif_5:
    jmp endif_1
else_0:

    ; output
    mov eax, 0
    push eax
    pop eax
    push eax
    push OFFSET fmt_int
    call printf
    add esp, 8
endif_1:

    ; return
    mov eax, 0
    push eax
    pop eax
    pop edi
    pop esi
    pop ebx
    mov esp, ebp
    pop ebp
    ret
main ENDP

end main
