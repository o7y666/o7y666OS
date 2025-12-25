; o7y666-os
; TAB=4
		CYLS	EQU		10

		ORG		0x7c00			; BIOS 将启动扇区（第一个扇区）的内容加载到内存地址0x7c00处
								; 在这之后的代码的数据都是从0x7c00开始
								; 这不是CPU指令，是汇编器的伪指令，不生产机器码只影响后续指令的地址计算

		;标准FAT12格式的软盘
		JMP		entry			;生成机器码：操作码＋偏移量
		DB		0x90
		DB		"HAMSTER "		; 启动区的名字 必须8字节
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
		DB		"HAMSTEROS  "	; 磁盘名字11字节
		DB		"FAT12   "		; 磁盘格式名称 8字节
		RESB	18				; 空出8字节

; 主程序入口 初始化栈、段寄存器
entry:
		MOV		AX,0			; 置零
		MOV		SS,AX			; ss 栈段寄存器，定义栈的起始地址
		MOV		SP,0x7c00		; SP 栈指针寄存器，指向栈顶 栈设置：SS:SP = 0000:7C00，栈向低地址增长
		MOV		DS,AX			; DS = 0 (数据段寄存器，用于数据访问)
		;一张软盘有80个柱面，18个扇区，两个磁头。一个扇区为512Byte大小。一张软盘的容量就是：80×2×18×512=1440Byte
		MOV		AX,0x0820		;避免和其他区域冲突 表示数据从软盘读出之后要放到内存的什么位置
		MOV		ES,AX			;ES是起辅助作用的段寄存器，一开始只有BX(16BIT)不能满足内存需求，最多只能用64K内存
								;EBX是后来才有的，因此在此之前，为了解决内存问题，用ES×16+BX来表示内存地址，这样内存能达到1M
		MOV		CH,0			;柱面号
		MOV		DH,0			;磁头号
		MOV		CL,2			;扇区号
		MOV		SI,0
readloop:
		MOV		SI,0
retry:
		MOV    	AH,0x02			;读取磁盘
		MOV		AL,1			;处理对象的扇区数，只能连续读取
		MOV		BX,0
		MOV		DL,0x00			;驱动器号
		INT		0x13			;BOIS函数 数据读取到0x8200-0x83FF
		JNC		next		    ;JNC 没有进位就跳转

		ADD 	SI,1
		CMP		SI,5
		JAE		error			;JAE 大于等于时跳转

		MOV		AH,0x00
		MOV		DL,0x00
		INT		0x13
		JMP		retry			;无条件跳转
;读剩下的扇区
next:
		MOV		AX,ES			;不能直接ADD ES,0x0020,因此需要用AX中转
		ADD		AX,0x0020
		MOV		ES,AX
		ADD		CL,1
		CMP		CL,18
		JBE		readloop		;小于等于时跳转
;读完18个扇区后，开始读下一个磁头
		MOV		CL,1			
		ADD		DH,1
		CMP		DH,2
		JB		readloop		;小于2就跳转

		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop
; 跳转到hamster.sys
		JMP		0xc200

error:
		MOV		SI,msg
putloop:
		MOV		AL,[SI]			;AL = 当前字符 (AX的低8位，存放要显示的字符)
		ADD		SI,1			;SI++ (指向下一个字符)
		CMP		AL,0			;判断AL是否为0，字符串的结束标志为0
		JE		fin				;jump equal
		MOV		AH,0x0e			; AH = 0x0E (AX的高8位，BIOS功能号：电传打字输出)
		MOV		BX,15			; BX = 15 (基址寄存器，这里设置显示属性，但通常被忽略)
		INT		0x10			; 调用BIOS中断显示字符
		JMP		putloop			;继续循环
fin:
		HLT						; 暂停CPU执行
		JMP		fin				; 
msg:
		DB		0x0a, 0x0a		; 换行2次
		DB		"load error"
		DB		0x0a			; 
		DB		0				;结束标志

		RESB	0x7dfe-$		; 
		;引导扇区结束标志
		DB		0x55, 0xaa		;这段代码就是扇区1中的内容
