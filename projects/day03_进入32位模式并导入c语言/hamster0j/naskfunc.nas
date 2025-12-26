; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式 windows COFF
[BITS 32]						; 制作32位模式用的机械语言


;制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt			; 程序中包含的函数名

;以下是实际的函数
[SECTION .text]		; SECTION 表示定义一个内存段 .text 表示存放程序代码
					; 相当于告诉编译器后面的内容都是程序指令，请放到代码区域

_io_hlt:	; void io_hlt(void);
		HLT
		RET
