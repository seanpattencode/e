; easm — e's startup path in pure x86-64 Linux asm. No libc, no ld.so, ~11 syscalls total.
; raw mode + winsize + render file + inverse modeline + wait key + restore + clear.
; build: nasm -f elf64 easm.s -o /tmp/easm.o && ld -s -o easm /tmp/easm.o
BITS 64
%define FBUF 1048576

section .bss
told:  resb 64                  ; kernel termios (36 bytes used)
tnew:  resb 64
wsz:   resb 8
keyb:  resb 8
fbuf:  resb FBUF
obuf:  resb FBUF*2

section .data
ebuf:  db 27,'[1;1H',0,0        ; echo frame: cursor home + typed char

section .rodata
s_clr: db 27,'[2J',27,'[H'
L_CLR  equ $-s_clr
s_inv: db 27,'[7m easm '
L_INV  equ $-s_inv
s_off: db ' ',27,'[K',27,'[m',27,'[H'
L_OFF  equ $-s_off
s_bye: db 27,'[m',27,'[2J',27,'[H'
L_BYE  equ $-s_bye

section .text
global _start
_start:
    xor r14d, r14d
    cmp qword [rsp], 1
    jle .args_done
    mov r14, [rsp+16]           ; argv[1]
.args_done:

    mov eax, 16                 ; ioctl TIOCGWINSZ
    mov edi, 1
    mov esi, 0x5413
    lea rdx, [wsz]
    syscall
    movzx r12d, word [wsz]      ; rows
    movzx r13d, word [wsz+2]    ; cols
    test r12d, r12d
    jnz .sz
    mov r12d, 24
    mov r13d, 80
.sz:
    cmp r12d, 256
    jbe .rok
    mov r12d, 256
.rok:
    cmp r13d, 512
    jbe .cok
    mov r13d, 512
.cok:

    mov eax, 16                 ; ioctl TCGETS
    mov edi, 1
    mov esi, 0x5401
    lea rdx, [told]
    syscall
    mov rax, [told]
    mov [tnew], rax
    mov rax, [told+8]
    mov [tnew+8], rax
    mov rax, [told+16]
    mov [tnew+16], rax
    mov rax, [told+24]
    mov [tnew+24], rax
    mov rax, [told+32]
    mov [tnew+32], rax
    and dword [tnew+12], ~0xB   ; c_lflag &= ~(ISIG|ICANON|ECHO)
    mov eax, 16                 ; ioctl TCSETS
    mov edi, 1
    mov esi, 0x5402
    lea rdx, [tnew]
    syscall

    xor r15, r15                ; file length
    test r14, r14
    jz .render
    mov eax, 2                  ; open(argv1, O_RDONLY)
    mov rdi, r14
    xor esi, esi
    syscall
    test eax, eax
    js .render
    mov edi, eax
    xor eax, eax                ; read(fd, fbuf, FBUF)
    lea rsi, [fbuf]
    mov edx, FBUF
    syscall
    test rax, rax
    js .closef
    mov r15, rax
.closef:
    mov eax, 3                  ; close(fd) — rdi survives syscalls
    syscall

.render:
    lea rdi, [obuf]
    lea rsi, [s_clr]
    mov ecx, L_CLR
    rep movsb

    lea rsi, [fbuf]
    lea r8, [fbuf]
    add r8, r15
    mov r9d, r12d
    dec r9d                     ; rows-1 text rows
.line:
    test r9d, r9d
    jz .mode
    cmp rsi, r8
    jae .mode
    mov ecx, r13d               ; column budget
.lcopy:
    cmp rsi, r8
    jae .leol
    lodsb
    cmp al, 10
    je .leol
    cmp al, 13
    je .lcopy
    test ecx, ecx
    jz .lskip
    stosb
    dec ecx
    jmp .lcopy
.lskip:                         ; truncated: swallow rest of line
    cmp rsi, r8
    jae .leol
    lodsb
    cmp al, 10
    jne .lskip
.leol:
    mov ax, 0x0A0D              ; \r\n
    stosw
    dec r9d
    jmp .line

.mode:                          ; cursor to last row: ESC [ rows ; 1 H
    mov al, 27
    stosb
    mov al, '['
    stosb
    mov eax, r12d               ; itoa rows (1..256)
    mov ebx, 100
    xor edx, edx
    div ebx
    test eax, eax
    jz .tens
    add al, '0'
    stosb
    mov eax, edx
    mov ebx, 10
    xor edx, edx
    div ebx
    add al, '0'
    stosb
    mov al, dl
    add al, '0'
    stosb
    jmp .rowdone
.tens:
    mov eax, edx
    mov ebx, 10
    xor edx, edx
    div ebx
    test eax, eax
    jz .ones
    add al, '0'
    stosb
.ones:
    mov al, dl
    add al, '0'
    stosb
.rowdone:
    mov al, ';'
    stosb
    mov ax, '1H'
    stosw
    lea rsi, [s_inv]
    mov ecx, L_INV
    rep movsb
    test r14, r14
    jz .noname
    mov rsi, r14
    mov ecx, 64
.fname:
    lodsb
    test al, al
    jz .noname
    stosb
    dec ecx
    jnz .fname
.noname:
    lea rsi, [s_off]
    mov ecx, L_OFF
    rep movsb

    lea rsi, [obuf]             ; single write: whole frame
    mov rdx, rdi
    sub rdx, rsi
    mov eax, 1
    mov edi, 1
    syscall

.keyloop:                       ; typing floor: read key, echo minimal render; q/EOF quits
    xor eax, eax
    xor edi, edi
    lea rsi, [keyb]
    mov edx, 1
    syscall
    test rax, rax
    jle .quit
    cmp byte [keyb], 'q'
    je .quit
    mov al, [keyb]
    mov [ebuf+6], al
    mov eax, 1
    mov edi, 1
    lea rsi, [ebuf]
    mov edx, 7
    syscall
    jmp .keyloop
.quit:
    mov eax, 16                 ; restore termios
    mov edi, 1
    mov esi, 0x5402
    lea rdx, [told]
    syscall
    mov eax, 1
    mov edi, 1
    lea rsi, [s_bye]
    mov edx, L_BYE
    syscall

    mov eax, 60                 ; exit(0)
    xor edi, edi
    syscall
