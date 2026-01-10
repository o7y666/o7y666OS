; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式 windows COFF
[BITS 32]						; 制作32位模式用的机械语言
[INSTRSET "i486p"]


;制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt 	; 	
		GLOBAL	_io_in8, _io_in16, _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags

;以下是实际的函数
[SECTION .text]		; SECTION 表示定义一个内存段 .text 表示存放程序代码
					; 相当于告诉编译器后面的内容都是程序指令，请放到代码区域

_io_hlt:	; void io_hlt(void); ;HLT 表示halt（暂停），等待中断唤醒CPU
		HLT							
		RET

_io_cli:	; clean interrupt flag(清除中断标志) 将EFLAGS 寄存器中的 IF（Interrupt Flag）位清零（设为 0），从而禁止可屏蔽中断（maskable interrupts）
		CLI
		RET

_io_sti:	; Set interrupt Flag (设置中断标志)
		STI
		RET

_io_stihlt:	;先开启中断（以防永远无法唤醒），然后 halt 等待中断。
		STI
		HLT
		RET

_io_in8:   ;int io_in8(int port)
		MOV 	EDX	, [ESP+4]    ;port
		MOV		EAX , 0
		IN		AL,	DX
		RET

_io_in16:   ;int io_in16(int port)
		MOV 	EDX	, [ESP+4]    ;port
		MOV		EAX , 0
		IN		AX,	DX
		RET

_io_in32:   ;int io_in32(int port)
		MOV 	EDX	, [ESP+4]    ;port
		IN		EAX,	DX
		RET

_io_out8:   ;void io_out8(int port, int data)
		MOV 	EDX	, [ESP+4]    ;port
		MOV		AL ,  [ESP+8]	 ;data 
		OUT		DX,	AL
		RET

_io_out16:   ;void io_out16(int port, int data)
		MOV 	EDX	, [ESP+4]    
		MOV		EAX ,  [ESP+8]	 
		OUT		DX,	AX
		RET

_io_out32:   ;void io_out32(int port, int data)
		MOV 	EDX	, [ESP+4]    
		MOV		EAX , [ESP+8]	 
		OUT		DX,	EAX
		RET

_io_load_eflags:	;void io_load_eflags(void)
		PUSHFD		;指 push EFLAGS
		POP	EAX
		RET

_io_store_eflags:	;void io_store_eflags(int flags)
		MOV		EAX, [ESP+4]
		PUSH	EAX
		POPFD	;指pop eflags
		RET
