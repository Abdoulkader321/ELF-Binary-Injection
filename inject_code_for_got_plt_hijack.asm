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

  ; exit
  mov rax, 60 ; System call number (exit)
  mov rdi, 0  ; success status code
  syscall     ; kernel call


  ; load context
  pop r11
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rax

message: db "Je suis trop un hacker", 0xA, 0x0
len: dd $-message