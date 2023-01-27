; ** Helper Procedures **
Power:
    push ebp
    mov ebp, esp
	push ebx
    mov eax, [ebp+12]
    mov ebx, [ebp+8]
    cmp ebx, 0
    je p_zero
    test ebx, 1
    jz p_even
	push eax
    mul eax
    push eax
	dec ebx
    shr ebx, 1
    push ebx
    call Power
	pop ebx
    mul ebx
    jmp p_end
p_zero:
    mov eax, 1
    jmp p_end
p_even:
    mul eax
    push eax
	shr ebx, 1
    push ebx
    call Power
p_end:
	pop ebx
    pop ebp
    ret 8
; Power

Fact:
	push ebp
	mov ebp, esp
	push ebx
	mov ebx, [ebp+8]
	cmp ebx, 1
	jle f_one
	push ebx
	dec ebx
	push ebx
	call Fact
	pop ebx
	mul ebx
	jmp f_done
f_one:
	mov eax, 1
f_done:
	pop ebx
	pop ebp
	ret 4
; Fact

Print:
    push ebp
    mov ebp, esp
    pusha
    mov eax, 4
    mov ebx, 1
    mov ecx, [ebp+8]
    mov edx, [ebp+12]
    int 0x80
    popa
    pop ebp
    ret 8
; Print

PrintNewLine:
	pusha
	mov eax, 4
    mov ebx, 1
    mov ecx, new_line_chr
    mov edx, 1
    int 0x80
	popa
	ret
; PrintNewLine

PrintInt:
    push ebp
    mov ebp, esp
    pusha
    mov eax, [ebp+8]
    mov esi, out_buf+out_buf_len-1
    mov ebx, 10
    mov byte [esi], '0'
    mov edi, 0
    cmp eax, 0
    je print_num
    inc esi
    cmp ax, 0
    jge conv_loop
    neg eax
    mov edi, 1
conv_loop:
    cmp eax, 0
    je end_conv
    dec esi
    xor edx, edx
    div ebx
    add edx, '0'
    mov [esi], dl
    jmp conv_loop
end_conv:
    cmp edi, 0
    je print_num
    dec esi
    mov byte [esi], '-'
print_num:
    mov edx, out_buf+out_buf_len
    sub edx, esi
    push edx
    push esi
    call Print
    popa
    pop ebp
    ret 4
; PrintInt