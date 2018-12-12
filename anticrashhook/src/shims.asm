wrapper_load PROTO C

.CODE
  shim_crash PROC
  int 3
  nop; nop; nop; nop; nop; nop; nop
  shim_crash ENDP

MAKE_SHIM macro name, ordinal
  .DATA
  PUBLIC shim_p_&name&
  shim_p_&name& QWORD shim_crash
ENDM
INCLUDE shims.inc
PURGE MAKE_SHIM

MAKE_SHIM macro name, ordinal
  .CODE
  shim_l_&name& PROC
    push r11
    push r10
    push r9
    push r8
    push rdx
    push rcx
    push rax
    call wrapper_load
    pop rax
    pop rcx
    pop rdx
    pop r8
    pop r9
    pop r10
    pop r11
    jmp shim_p_&name&
  shim_l_&name& ENDP
ENDM
INCLUDE shims.inc
PURGE MAKE_SHIM

MAKE_SHIM macro name, ordinal
  .CODE
  shim_h_&name& PROC
    jmp shim_p_&name&
  shim_h_&name& ENDP
ENDM
INCLUDE shims.inc
PURGE MAKE_SHIM

END
