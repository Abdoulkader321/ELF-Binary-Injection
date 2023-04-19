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

  mov rdi, 1 ; stdout
  lea rsi, [rel string] ; message address
  mov rdx, [rel len] ; message length
  mov rax, 1 ; write syscall number
  syscall


  ; load context
  pop r11
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rax

  push 0x4022e0
  ret


string: db "Je suis trop un hacker", 10, 0
len: dd $-string