;hamster-os
;TAB=4

; 有关BOOT_INFO
CYLS	EQU			0x0ff0		;设定启动区
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
fin:
		HLT
		JMP			fin
