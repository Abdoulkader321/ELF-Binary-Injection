BITS 64

SECTION .text
global main

main:
  ; save context
  push rax
  push rcx
  push rdx
  push rsi
  push rdi
  push r11

  ; write
  mov rax, 1              ; System call number (write)
  mov rdi, 1              ; stdout
  lea rsi, [rel message]  ; message to display
  mov rdx, [rel len]      ; message length
  syscall                 ; kernel call

  ; load context
  pop r11
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rax

  ; call original entry point
  push 0x4022e0
  ret


message: db "Je suis trop un hacker", 0xA, 0x0
len: dd $-message