; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式 windows COFF
[BITS 32]						; 制作32位模式用的机械语言
[INSTRSET "i486p"]


;制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt,_write_mem8		; 程序中包含的函数名		

;以下是实际的函数
[SECTION .text]		; SECTION 表示定义一个内存段 .text 表示存放程序代码
					; 相当于告诉编译器后面的内容都是程序指令，请放到代码区域

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_write_mem8: ;void write_mem8(int addr, int data) 往指定地址写入数据
		MOV		ECX, [ESP+4] ;[ESP+4]中存放的是地址，将其读入ECX中
		MOV		AL, [ESP+8]  ;[ESP+8]中存放的是数据，将其读入AL
		MOV		[ECX], AL
		RET
