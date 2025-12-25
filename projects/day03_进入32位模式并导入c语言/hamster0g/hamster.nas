;hamster-os
;TAB=4
		ORG			0xc200		;程序将被装载到的地址
		
		MOV			AL,0x13 	;分辨率320×200 颜色深度：8位 256色
		MOV			AH,0x00
		INT			0x10
fin:
		HLT
		JMP			fin
