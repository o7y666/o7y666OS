; o7y666-os
; TAB=4

		ORG		0x7c00			; BIOS 将启动扇区（第一个扇区）的内容加载到内存地址0x7c00处
								; 在这之后的代码的数据都是从0x7c00开始
								; 这不是CPU指令，是汇编器的伪指令，不生产机器码只影响后续指令的地址计算

		;标准FAT12格式的软盘
		JMP		entry			;生成机器码：操作码＋偏移量
		DB		0x90
		DB		"o7yIPL  "		; 启动区的名字 必须8字节
		DW		512				; 每个扇区的大小
		DB		1				; 簇的大小 必须为一个扇区
		DW		1			    ; FAT的起始位置
		DB		2				; FAT的个数
		DW		224				; 根目录的大小
		DW		2880			; 该磁盘的大小 必须是2880个扇区
		DB		0xf0			; 磁盘的种类
		DW		9				; FAT的长度
		DW		18				; 一个磁道有几个扇区
		DW		2				; 磁头数
		DD		0				; 不使用分区
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 固定
		DD		0xffffffff		; 
		DB		"o7y666-OS  "	; 磁盘名字11字节
		DB		"FAT12   "		; 磁盘格式名称 8字节
		RESB	18				; 空出8字节

; 

entry:
		MOV		AX,0			; 
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		MOV		ES,AX

		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 
		MOV		BX,15			; 
		INT		0x10			; 
		JMP		putloop
fin:
		HLT						; 
		JMP		fin				; 

msg:
		DB		0x0a, 0x0a		; 
		DB		"i love ozy"
		DB		0x0a			; 
		DB		0

		RESB	0x7dfe-$		; 

		DB		0x55, 0xaa
