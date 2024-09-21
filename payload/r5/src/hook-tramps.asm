.intel_syntax

.macro push_gp
 pushfq
 push rax
 push rbx
 push rcx
 push rdx
 push rsi
 push rdi
 push rbp
 push r8
 push r9
 push r10
 push r11
 push r12
 push r13
 push r14
 push r15
.endm

.macro pop_gp
 pop r15
 pop r14
 pop r13
 pop r12
 pop r11
 pop r10
 pop r9
 pop r8
 pop rbp 
 pop rdi
 pop rsi
 pop rdx
 pop rcx
 pop rbx
 pop rax
 popfq

.endm

.global tp_mkrctx_unk1
tp_mkrctx_unk1:
    push_gp
    sub rsp, 0x28
    
    mov r8, qword ptr [rsp+0x80+0x28]
    call hk_mkrctx_unk1
    
    add rsp, 0x28
    pop_gp

    jmp qword ptr [rip+ghk_orig_mkrctx_unk1]

.global tp_on_receive_server_info
tp_on_receive_server_info:
    push_gp
    sub rsp, 0x28

    call hk_on_receive_server_info
    
    add rsp, 0x28
    pop_gp

    jmp qword ptr [rip+ghk_orig_on_receive_server_info]

.global tp_on_dropped_by_server
tp_on_dropped_by_server:
    push_gp
    sub rsp, 0x28
    
    call hk_on_dropped_by_server
    
    add rsp, 0x28
    pop_gp

    jmp qword ptr [rip+ghk_orig_on_dropped_by_server]

.global tp_process_playlist_override
tp_process_playlist_override:
    push_gp
    sub rsp, 0x28
    
    call hk_process_playlist_override
    
    add rsp, 0x28
    pop_gp

    jmp qword ptr [rip+ghk_orig_process_playlist_override]

.global tp_init
tp_init:
    push_gp
    sub rsp, 0x28
    
    call main
    
    add rsp, 0x28
    pop_gp

    xor rax, rax
    lea rax, qword ptr [rip+gctx]
    mov rax, qword ptr [rax]

    jmp qword ptr [rax+0x5]

.global tp_override_mouse
tp_override_mouse:
    push_gp
    sub rsp, 0x28

    mov rdx, qword ptr [rsp+0x80+0x28]
    call hk_override_mouse
    add rsp, 0x28
    pop_gp

    jmp qword ptr [rip+ghk_orig_override_mouse]

