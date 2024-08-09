
.data
maxFuncSize dq 80h
overflowCheckDN dq 7FFFFFFFh
overflowCheckUP dq 0FFFFFFFF80000000h
.CODE

; ================================================
; RCX = func
; EDX = pattern

asmPatchUtils_findPattern32 proc 
	;========[ push all afflicted registers ]
	push r9
	;========[ main code ]
	mov rax, -1
loopStart:
	cmp rax, maxFuncSize
	jge limitReached
	inc rax
	lea r9, [rcx+rax]
	mov r9d, [r9] 
	sub r9d, edx
	jnz loopStart
endfunc:
	;========[ pop all afflicted registers ]
	pop r9
	ret ; rax = size
limitReached:
	;========[ pop all afflicted registers ]
	pop r9
	mov rax, -1
	ret
asmPatchUtils_findPattern32 endp

; ================================================
; RCX = funcToInject
; RDX = funcInjectionAddr
; R8 = valueToCalc

asmPatchUtils_formatNextRelative PROC 
	;========[ push all afflicted registers ]
	push rbx
	;========[ main code ]
	push rdx
	xor rdx, rdx
	mov rdx, 0DEADC0DEh
	call asmPatchUtils_findPattern32	; get EndOfFunc

	cmp rax, -1 ; check if code limit reached
	je out_of_range
	
	mov rbx, rax ; save EndOfFunc offset
	mov rdx, 0DEADBEEFh
	call asmPatchUtils_findPattern32	; find next RelPattern

	cmp rax, rbx ; check if code limit reached (found outside)
	jge out_of_range
	cmp rax, -1	; check if code limit reached
	je out_of_range


	; rax = relPatternOffset
	pop rdx
	mov rbx, rax ; save offset
	add rax, 4
	add rax, rdx
	sub r8, rax

	; check 32 bit overflow
	cmp r8, overflowCheckDN
	jg out_of_range
	cmp r8, overflowCheckUP
	jl out_of_range

	; r8 = true relative addr
	lea rax, [rcx + rbx]
	mov [rax], r8d	; patch instruction with new calculated relative value
	mov rax, 1		; return success
	;========[ pop all afflicted registers ]
	pop rbx
	ret

out_of_range:
	pop rax
	mov rax, -1
	;========[ pop all afflicted registers ]
	pop rbx
	ret

asmPatchUtils_formatNextRelative ENDP

END
