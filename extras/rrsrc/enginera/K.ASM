.386P

EXTRN _kdmasm1 : dword
EXTRN _kdmasm2 : dword
EXTRN _kdmasm3 : dword
EXTRN _kdmasm4 : dword

EXTRN _chainbackcnt : dword
EXTRN _chainbackstart : dword
EXTRN _pcsndptr : dword
EXTRN _samplecount : dword
EXTRN _pcsndbufsiz : dword
EXTRN _oldpctimerhandler : far
EXTRN _bufferside : byte

EXTRN _qualookup : near
EXTRN _pcsndlookup : near
EXTRN _chain_intr_ : near
EXTRN __GETDS : near
EXTRN preparesndbuf_ : near

_DATA SEGMENT BYTE PUBLIC USE32 'DATA'
_DATA ENDS
DGROUP GROUP _DATA

CODE SEGMENT BYTE PUBLIC USE32 'CODE'
ASSUME cs:CODE

PUBLIC monolocomb_
monolocomb_:
	;eax=temp2,ebx=temp1,ecx=cnt,edx=dat,esi=dasplc,edi=stemp,ebp=dasoff
	push ebp

	mov dword ptr ds:[mach1lm+3], ebx        ;voloffs1

	mov ebx, _kdmasm4
	mov dword ptr ds:[mach5lm+2], ebx

	mov ebx, edx
	shl ebx, 32-12
	sar edx, 12
	mov dword ptr ds:[mach3lm+2], ebx        ;dasinc
	mov dword ptr ds:[mach4lm+2], edx        ;dasinc

	mov ebp, esi
	shl ebp, 32-12
	sar esi, 12

	xor edx, edx
	jmp begitlm

prebegitlm:
	cmp _kdmasm1, 0
	jz enditlm
	mov ebx, _kdmasm2
	mov dword ptr ds:[mach5lm+2], ebx
	mov _kdmasm4, ebx
	mov ebx, _kdmasm3
	mov eax, ebx
	shl ebx, 32-12
	sar eax, 12
	sub ebp, ebx
	sbb esi, eax
	test ecx, ecx
	jz enditlm

begitlm:
		;dl = [(eax>>12)+dasoff];     (QUICK NO MULTIPLY METHOD)
mach5lm: mov dl, byte ptr [esi+88888888h]      ;dasoff

	mov ebx, dword ptr [edi]
	add edi, 4
mach1lm: add ebx, dword ptr [edx*4+88888888h]  ;voloffs1
mach3lm: add ebp, 88888888h                    ;dasinc
mach4lm: adc esi, 88888888h                    ;dasinc
	mov dword ptr [edi-4], ebx
	dec ecx

	ja begitlm          ;jump if (no carry (add)) and (not zero (dec))!
	jc prebegitlm
enditlm:

	shl esi, 12
	shr ebp, 32-12
	lea eax, [esi+ebp]

	pop ebp
	ret

PUBLIC monohicomb_
monohicomb_:
	;eax=temp2,ebx=temp1,ecx=cnt,edx=dat,esi=dasplc,edi=stemp,ebp=dasoff
	push ebp
	mov ebp, ecx

	mov dword ptr ds:[mach1hm+3], ebx        ;voloffs1

	mov ebx, _kdmasm4
	mov dword ptr ds:[mach5hm+3], ebx

	mov ebx, edx
	shl ebx, 32-12
	sar edx, 12
	mov dword ptr ds:[mach4hm+2], edx        ;dasinc
	;mov edx, ebx       ;Fake 4-bit counter to save shift
	;shr edx, 20        ;
	;mov bx, dx         ;
	mov dword ptr ds:[mach3hm+2], ebx        ;dasinc

	mov ecx, esi
	shl ecx, 32-12
	sar esi, 12
	;mov edx, ecx       ;Fake 4-bit counter to save shift
	;shr edx, 20        ;
	;mov cx, dx         ;

	xor edx, edx
	sub edi, 4
	jmp begithm

prebegithm:
	cmp _kdmasm1, 0
	jz endithm
	mov ebx, _kdmasm2
	mov dword ptr ds:[mach5hm+3], ebx
	mov _kdmasm4, ebx
	mov ebx, _kdmasm3
	mov eax, ebx
	shl ebx, 32-12
	sar eax, 12
	sub ecx, ebx
	sbb esi, eax
	test ebp, ebp
	jz endithm

begithm:
	;edx = _qualookup[((ebp>>19)&0x00001e00)+(bh-bl)]+bl
mach5hm: mov bx, word ptr [esi+88888888h]
	mov eax, ecx
	shr eax, 20
	add edi, 4
	mov dl, bl
	mov al, bl
	sub al, bh
	adc ah, ah
	add dl, byte ptr _qualookup[eax]

	mov ebx, dword ptr [edi]
mach1hm: add ebx, dword ptr [edx*4+88888888h]  ;voloffs1
mach3hm: add ecx, 88888888h                    ;dasinc
mach4hm: adc esi, 88888888h                    ;dasinc
	dec ebp

	mov dword ptr [edi], ebx

	ja begithm          ;jump if (no carry (add)) and (not zero (dec))!
	jc prebegithm
endithm:

	shl esi, 12
	shr ecx, 32-12
	lea eax, [esi+ecx]

	pop ebp
	ret

PUBLIC stereolocomb_
stereolocomb_:
	;eax=temp2,ebx=temp1,ecx=cnt,edx=dat,esi=dasplc,edi=stemp,ebp=dasoff
	push ebp

	mov dword ptr ds:[mach1ls+3], ebx        ;voloffs1
	add ebx, 4
	mov dword ptr ds:[mach2ls+3], ebx        ;voloffs2

	mov ebx, _kdmasm4
	mov dword ptr ds:[mach5ls+2], ebx

	mov ebx, edx
	shl ebx, 32-12
	sar edx, 12
	mov dword ptr ds:[mach3ls+2], ebx        ;dasinc
	mov dword ptr ds:[mach4ls+2], edx        ;dasinc

	mov ebp, esi
	shl ebp, 32-12
	sar esi, 12

	xor edx, edx
	sub edi, 8
	jmp begitls

prebegitls:
	cmp _kdmasm1, 0
	jz enditls
	mov ebx, _kdmasm2
	mov dword ptr ds:[mach5ls+2], ebx
	mov _kdmasm4, ebx
	mov ebx, _kdmasm3
	mov eax, ebx
	shl ebx, 32-12
	sar eax, 12
	sub ebp, ebx
	sbb esi, eax
	test ecx, ecx
	jz enditls
	jmp begitls

begitls:
	add edi, 8

		;dl = [(eax>>12)+dasoff];     (QUICK NO MULTIPLY METHOD)
mach5ls: mov dl, byte ptr [esi+88888888h]      ;dasoff

	mov ebx, dword ptr [edi]
	mov eax, dword ptr [edi+4]

mach1ls: add ebx, dword ptr [edx*8+88888888h]  ;voloffs1
mach2ls: add eax, dword ptr [edx*8+88888888h]  ;voloffs2

mach3ls: add ebp, 88888888h                    ;dasinc
	mov dword ptr [edi], ebx
mach4ls: adc esi, 88888888h                    ;dasinc
	dec ecx

	mov dword ptr [edi+4], eax

	ja begitls          ;jump if (no carry (add)) and (not zero (dec))!
	jc prebegitls
enditls:

	shl esi, 12
	shr ebp, 32-12
	lea eax, [esi+ebp]

	pop ebp
	ret

PUBLIC stereohicomb_
stereohicomb_:
	;eax=temp2,ebx=temp1,ecx=cnt,edx=dat,esi=dasplc,edi=stemp,ebp=dasoff
	push ebp
	mov ebp, ecx

	mov dword ptr ds:[mach1hs+3], ebx        ;voloffs1
	add ebx, 4
	mov dword ptr ds:[mach2hs+3], ebx        ;voloffs2

	mov ebx, _kdmasm4
	mov dword ptr ds:[mach5hs+3], ebx

	mov ebx, edx
	shl ebx, 32-12
	sar edx, 12
	mov dword ptr ds:[mach4hs+2], edx        ;dasinc
	;mov edx, ebx       ;Fake 4-bit counter to save shift
	;shr edx, 20        ;
	;mov bx, dx         ;
	mov dword ptr ds:[mach3hs+2], ebx        ;dasinc

	mov ecx, esi
	shl ecx, 32-12
	sar esi, 12
	;mov edx, ecx       ;Fake 4-bit counter to save shift
	;shr edx, 20        ;
	;mov cx, dx         ;

	xor edx, edx
	sub edi, 8
	jmp begiths

prebegiths:
	cmp _kdmasm1, 0
	jz endiths
	mov ebx, _kdmasm2
	mov dword ptr ds:[mach5hs+3], ebx
	mov _kdmasm4, ebx
	mov ebx, _kdmasm3
	mov eax, ebx
	shl ebx, 32-12
	sar eax, 12
	sub ecx, ebx
	sbb esi, eax
	test ebp, ebp
	jz endiths

begiths:
	;edx = _qualookup[((ebp>>19)&0x00001e00)+(bh-bl)]+bl
mach5hs: mov bx, word ptr [esi+88888888h]
	mov eax, ecx
	shr eax, 20
	add edi, 8
	mov dl, bl
	mov al, bl
	sub al, bh
	adc ah, ah
	add dl, byte ptr _qualookup[eax]

	mov eax, dword ptr [edi]
mach1hs: mov ebx, dword ptr [edx*8+88888888h]  ;voloffs1
	add ebx, eax
	mov eax, dword ptr [edi+4]
	mov dword ptr [edi], ebx
mach2hs: mov ebx, dword ptr [edx*8+88888888h]  ;voloffs2
	add eax, ebx

mach3hs: add ecx, 88888888h                    ;dasinc
mach4hs: adc esi, 88888888h                    ;dasinc

	dec ebp

	mov dword ptr [edi+4], eax

	ja begiths          ;jump if (no carry (add)) and (not zero (dec))!
	jc prebegiths
endiths:

	shl esi, 12
	shr ecx, 32-12
	lea eax, [esi+ecx]

	pop ebp
	ret

PUBLIC setuppctimerhandler_
setuppctimerhandler_:
	mov dword ptr ds:[pcmach3+2], eax

	cmp ebx, 65000d
	jl short skipsetuppc1
	mov word ptr ds:[pcmach1+0], 0c381h     ;81c300000000 = add ebx, 00000000h
	mov dword ptr ds:[pcmach1+2], 00000000h
	mov word ptr ds:[pcmach2+0], 0c98bh     ;8bc9 = mov ecx, ecx
skipsetuppc1:
	ret

PUBLIC pctimerhandler_ 
pctimerhandler_:
	push eax
	mov ax, DGROUP
	push ds
	mov ds, ax
	push edx

	mov edx, dword ptr _pcsndptr
	mov al, byte ptr [edx]
	out 42h, al
	inc edx
	mov dword ptr _pcsndptr, edx
pcmach1: dec _chainbackcnt              ;samplediv
pcmach2: jz short pc3
	mov al, 20h
	out 20h, al
pcmach3: cmp edx, 88888888h             ;pcsndptrend
	jge short pc1

	pop edx
	pop ds
	pop eax
	iretd

pc1: xor byte ptr _bufferside, 1
	mov eax, _pcsndbufsiz
	mov edx, _pcsndptr
	jnz short pc2
	sub edx, eax
	sub edx, eax
	mov _pcsndptr, edx
pc2: add eax, edx
	mov dword ptr ds:[pcmach3+2], eax
	sti
	call preparesndbuf_
	pop edx
	pop ds
	pop eax
	iretd

pc3: sti
	cmp edx, dword ptr ds:[pcmach3+2]
	jl short pc5

	xor byte ptr _bufferside, 1
	mov eax, _pcsndbufsiz
	mov edx, _pcsndptr
	jnz short pc4
	sub edx, eax
	sub edx, eax
	mov _pcsndptr, edx
pc4: add eax, edx
	mov dword ptr ds:[pcmach3+2], eax
	sti
	call preparesndbuf_
pc5:
	mov eax, _chainbackstart
	mov _chainbackcnt, eax
	pop edx
	pop ds
	pop eax
		;Must start interrupt handler like watcom c
	pushad
	push ds
	push es
	push fs
	push gs
	mov ebp, esp
	cld
	call __GETDS
	mov dx, word ptr [_oldpctimerhandler+4]
	mov eax, dword ptr [_oldpctimerhandler]
	call _chain_intr_

PUBLIC pcbound2char_
pcbound2char_:
	push ebp

	add ecx, ecx
	lea eax, [ecx+edi]
	mov dword ptr ds:[pcmachchar+3], eax
	xor edi, edi
	sub edi, ecx
	xor ecx, ecx

	xor edx, edx
	mov ebp, 0ffff0000h
startpcbound2char:
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [esi+4]
	test ebx, ebp
	jnz short pcboundit1
pcboundit1back:
	mov dl, bh
	mov dword ptr [esi], 32768
	test eax, ebp
	mov al, byte ptr _pcsndlookup[edx]
	jnz short pcboundit2
pcboundit2back:
	mov dl, ah
	mov dword ptr [esi+4], 32768
	mov ah, byte ptr _pcsndlookup[edx]
	add esi, 8
pcmachchar: mov word ptr [edi+88888888h], ax
	add edi, 2
	jnc short startpcbound2char
	pop ebp
	ret
pcboundit1:
	cmp ebx, 80000000h
	sbb bh, bh
	jmp short pcboundit1back
pcboundit2:
	cmp eax, 80000000h
	sbb ah, ah
	jmp short pcboundit2back

	;for(i=0;i<bytespertic;i++)
	;{
	;   j = (stemp[i]>>8);
	;   if (j < 0) j = 0;
	;   if (j > 255) j = 255;
	;   *charptr++ = (char)j;
	;}
PUBLIC bound2char_
bound2char_:
	add ecx, ecx
	lea eax, [ecx+edi]
	mov dword ptr ds:[machchar+3], eax
	xor edi, edi
	sub edi, ecx
	xor ecx, ecx

	mov edx, 0ffff0000h
startbound2char:
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [esi+4]
	test ebx, edx
	jnz short boundchar1
boundchar1back:
	mov dword ptr [esi], 32768
	test eax, edx
	jnz short boundchar2
boundchar2back:
	mov dword ptr [esi+4], 32768
	mov al, bh
	add esi, 8
machchar: mov word ptr [edi+88888888h], ax
	add edi, 2
	jnc short startbound2char
	ret
boundchar1:
	cmp ebx, 80000000h
	sbb bh, bh
	jmp short boundchar1back
boundchar2:
	cmp eax, 80000000h
	sbb ah, ah
	jmp short boundchar2back

	;for(i=0;i<bytespertic;i++)
	;{
	;   j = stemp[i];
	;   if (j < 0) j = 0;
	;   if (j > 65535) j = 65535;
	;   *shortptr++ = (short)(j^8000h);
	;}
PUBLIC bound2short_
bound2short_:
	shl ecx, 2
	lea eax, [ecx+edi]
	mov dword ptr ds:[machshort+2], eax
	xor edi, edi
	sub edi, ecx
	xor ecx, ecx

	mov edx, 0ffff0000h
startbound2short:
	mov ebx, dword ptr [esi]
	mov eax, dword ptr [esi+4]
	test ebx, edx
	jnz short boundshort1
boundshort1back:
	mov dword ptr [esi], 32768
	test eax, edx
	jnz short boundshort2
boundshort2back:
	shl eax, 16
	mov dword ptr [esi+4], 32768
	mov ax, bx
	add esi, 8
	xor eax, 80008000h
machshort: mov dword ptr [edi+88888888h], eax
	add edi, 4
	jnc short startbound2short
	ret
boundshort1:
	cmp ebx, 80000000h
	sbb ebx, ebx
	jmp short boundshort1back
boundshort2:
	cmp eax, 80000000h
	sbb eax, eax
	jmp short boundshort2back

CODE ENDS
END
