.data
.code
; ==============================[Patches]
patchSegment segment read write ; allow to modify this segment

patch_CalcMulAsAdd_first PROC
	cmp r14d, 05Ch ; 0x5C = 92 = MUL
	jne original
	mov r14d, 05Dh ; 0x5D = 93 = ADD
original:
	mov [rsi+018h], r14d
	lea rcx, [rsi + 0F70h]

	db 0E9h				; jmp back to orig func
	dd 0DEADBEEFh		; rel addr format
	dd 0DEADC0DEh		; end of func
	ret
patch_CalcMulAsAdd_first ENDP

patch_CalcMulAsAdd_second PROC
	cmp r14d, 05Ch ; 0x5C = 92 = MUL
	jne original
	mov r14d, 05Dh ; 0x5D = 93 = ADD
original:
	mov [rsi+018h], r14d
	cmp [rsi], r15b

	db 0E9h				; jmp back to orig
	dd 0DEADBEEFh		; rel addr format
	dd 0DEADC0DEh		; end of func
	ret
patch_CalcMulAsAdd_second ENDP

patchSegment ends 
; ================================================
END
