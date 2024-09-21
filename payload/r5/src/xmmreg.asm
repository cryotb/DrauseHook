.intel_syntax

.text
.global read_xmm7
read_xmm7:
    movaps xmm0, xmm7          
    movss dword ptr [rcx], xmm0
    ret

.global write_xmm7
write_xmm7:
    shufps xmm0, xmm0, 0   
    movaps xmm7, xmm0     
    ret

.global read_xmm6
read_xmm6:
    movaps xmm0, xmm6          
    movss dword ptr [rcx], xmm0
    ret

.global write_xmm6
write_xmm6:  
    shufps xmm0, xmm0, 0   
    movaps xmm6, xmm0     
    ret

.global read_xmm8
read_xmm8:
    movaps xmm0, xmm8          
    movss dword ptr [rcx], xmm0
    ret

.global write_xmm8
write_xmm8:  
    shufps xmm0, xmm0, 0   
    movaps xmm8, xmm0     
    ret

.global read_xmm15
read_xmm15:
    pextrq rax, xmm15, 1
    ret

.global write_xmm15
write_xmm15:
    pinsrq xmm15, rcx, 1
    ret
