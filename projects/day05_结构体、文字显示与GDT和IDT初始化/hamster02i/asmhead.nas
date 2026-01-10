; hamster-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; 
DSKCAC	EQU		0x00100000		; 
DSKCAC0	EQU		0x00008000		; 

; 有关BOOT_INFO
CYLS	EQU			0x0ff0		;
LEDS	EQU			0x0ff1
VMODE	EQU			0x0ff2		;关于颜色数目的信息，颜色位数
SCRNX	EQU			0x0ff4		;分辨率的X
SCRNY	EQU			0x0ff6		;分辨率的Y
VRAM	EQU			0x0ff8		;图像缓冲区的开始地址

		ORG			0xc200		;程序将被装载到的地址
		
		MOV			AL,0x13 	;分辨率320×200 颜色深度：8位 256色
		MOV			AH,0x00
		INT			0x10		;
		MOV			BYTE [VMODE],8 ;记录画面模式
		MOV			WORD [SCRNX],320
		MOV			WORD [SCRNY],200
		MOV			DWORD [VRAM],0X000a0000
;用BIOS取得键盘上各种LED指示灯的状态
		MOV			AH,0x02
		INT			0x16		;BIOS调用完之后寄存器AL的值会改变
		MOV			[LEDS],AL	;AL作为返回值存储键盘状态
;	

		MOV		AL,0xff			;屏蔽所有中断
		OUT		0x21,AL			;主PIC
		NOP						 
		OUT		0xa1,AL			;从PIC

		CLI						;禁止CPU级别中断 


;开启A20地址线，使得CPU可以访问1MB以上的内存
		CALL	waitkbdout		;等待键盘控制器空闲
		MOV		AL,0xd1
		OUT		0x64,AL			;发送命令到键盘控制器
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式

[INSTRSET "i486p"]				; 使用i486指令集

		LGDT	[GDTR0]			; 加载GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 禁用分页（确保PG位为0）
		OR		EAX,0x00000001	; 启用保护模式（PE位为1）
		MOV		CR0,EAX			;设置CR0，进入保护模式
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			;数据段选择子
		MOV		DS,AX			; 设置数据段寄存器
		MOV		ES,AX			; 设置附加段寄存器
		MOV		FS,AX			; 设置FS段寄存器
		MOV		GS,AX			; 设置GS段寄存器
		MOV		SS,AX			; 设置堆栈段寄存器

; 将bootpack（内核）复制到目标地址（0x280000）

		MOV		ESI,bootpack	; 源地址（bootpack在asmhead之后，由引导程序加载）
		MOV		EDI,BOTPAK		; 目标地址（0x280000）
		MOV		ECX,512*1024/4	;复制长度（512KB，以4字节为单位）
		CALL	memcpy

; 将磁盘数据复制到内存中

; 先复制引导扇区（0x7c00处）到磁盘缓存地址（0x100000）

		MOV		ESI,0x7c00		; 源地址（引导扇区）
		MOV		EDI,DSKCAC		; 目标地址（0x100000）
		MOV		ECX,512/4		;复制长度（512字节，以4字节为单位）
		CALL	memcpy

; 复制剩余的磁盘数据（从0x8000+512到0x100000+512）

		MOV		ESI,DSKCAC0+512	; 
		MOV		EDI,DSKCAC+512	; 
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 计算整个磁盘的数据量（柱面数*每个柱面的扇区数*512字节/4）
		SUB		ECX,512/4		; 减去已经复制的引导扇区（512字节）
		CALL	memcpy

; 完成复制后，跳转到内核执行

; 设置堆栈指针（ESP）并跳转到内核

		MOV		EBX,BOTPAK		; 内核地址（0x280000）
		MOV		ECX,[EBX+16]	; 从内核头部读取数据长度（可能是内核大小）
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 如果没有数据要复制，跳过
		MOV		ESI,[EBX+20]	; 源地址（内核内的偏移）
		ADD		ESI,EBX			;加上内核基址
		MOV		EDI,[EBX+12]	; 目标地址（可能是重定位地址）
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 设置堆栈指针（内核头部指定）
		JMP		DWORD 2*8:0x0000001b	;跳转到内核入口点（选择子为2 * 8，偏移0x1b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02		;检查输入缓冲区是否为空（位1为0表示空）
		JNZ		waitkbdout		; 
		RET
;内存复制
memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			;不为0则继续复制
		RET
; 对齐到16字节边界

		ALIGNB	16
GDT0:
		RESB	8				; 空描述符（8字节）
		DW		0xffff,0x0000,0x9200,0x00cf	;数据段描述符
		DW		0xffff,0x0000,0x9a28,0x0047	;代码段描述符

		DW		0
; GDT描述符表寄存器（GDTR）的值
GDTR0:
		DW		8*3-1		; GDT界限：3个描述符×8字节-1=23
		DD		GDT0		; GDT基地址

		ALIGNB	16			; 对齐到16字节边界
; bootpack内核数据的起始位置
bootpack:
