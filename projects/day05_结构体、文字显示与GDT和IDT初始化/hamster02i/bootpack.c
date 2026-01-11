#include<stdio.h>

// naskfunc 中用汇编语言写的函数
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

//本文件中定义的c语言函数
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int xsize, int ysize);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);


#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

struct BOOTINFO
{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

struct SEGMENT_DESCRIPTOR
{//limit 实际上是20位的，base是32位，ar是12位 8(访问权限字节+4位flags)
	short limit_low, base_low;
	char  base_mid, access_right;
	char  limit_high, base_high; //limit_hiht是个复合字段 高4位是flags，低4位是limit
};

struct GATE_DESCRIPTOR
{//offset 地址偏移量 selector 段选择子 dw_count 双字计数 access_right 访问权限
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void HariMain(void)
{	
	char s[40],mcursor[256];
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	int mx,my;
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;

	init_palette();//设定调色板
	init_screen(binfo->vram,binfo->scrnx,binfo->scrny);

	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram,binfo->scrnx,10,10,0,"abcdefg");
	putfonts8_asc(binfo->vram,binfo->scrnx,40,40,0,s);

	init_mouse_cursor8(mcursor,14);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);


	for (;;) {
		io_hlt();
	}
}

void init_palette(void)
{	
	// 16种颜色，每种颜色三个字节（R,G,B）
	static unsigned char table_rgb[16*3]={
		0x00, 0x00, 0x00,	//0:黑
		0xff, 0x00, 0x00,	//1:亮红
		0x00, 0xff, 0x00,	//2:亮绿
		0xff, 0xff, 0x00,	//3:亮黄
		0x00, 0x00, 0xff,	//4:亮蓝
		0xff, 0x00, 0xff,	//5:亮紫
		0x00, 0xff, 0xff,	//6:浅亮蓝
		0xff, 0xff, 0xff,	//7:白
		0xc6, 0xc6, 0xc6,	//8:亮灰
		0x84, 0x00, 0x00,	//9:暗红
		0x00, 0x84, 0x00,	//10:暗绿
		0x84, 0x84, 0x00,	//11:暗黄
		0x00, 0x00, 0x84,	//12:暗青
		0x84, 0x00, 0x84,	//13:暗紫
		0x00, 0x84, 0x84,	//14:浅暗蓝
		0x84, 0x84, 0x84,	//15:暗灰
	};
	set_palette(0,15,table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i,eflags;
	eflags=io_load_eflags(); //记录中断许可标志的值
	io_cli();				//将中断许可标志置为0，禁止中断
	io_out8(0x03c8, start); //往指定设备中传输数据 将调色板号写入0x03c8，紧接着按R,G,B顺序写入0x03c9
	for(i=start;i<=end;i++){
		io_out8(0x03c9,rgb[0]/4);//传统VGA硬件只支持6位颜色深度（0-63）
		io_out8(0x03c9,rgb[1]/4);
		io_out8(0x03c9,rgb[2]/4);
		rgb+=3;
	}
	io_store_eflags(eflags);
	return;
}

void boxfill8(unsigned 	char *vram ,int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x,y;
	for (y=y0;y<=y1;y++){
		for(x=x0;x<=x1;x++)
			vram[x+y*xsize]=c;
	}
	return;
}

void init_screen(char *vram, int xsize, int ysize)
{
	boxfill8(vram, xsize, 14, 0, 0, xsize-1, ysize-29);
	boxfill8(vram, xsize, 8, 0, ysize-29, xsize-1, ysize-28);
	boxfill8(vram, xsize, 7, 0, ysize-27, xsize-1, ysize-27);
	boxfill8(vram, xsize, 8, 0, ysize-26, xsize-1, ysize-1);
	boxfill8(vram, xsize, 7, 3, ysize-24, 59, ysize-24);
	boxfill8(vram, xsize, 7, 2, ysize-24, 2, ysize-4);
	boxfill8(vram, xsize, 15, 59, ysize-23, 59, ysize-5);
	boxfill8(vram, xsize, 15, 3, ysize-4, 59, ysize-4);
	boxfill8(vram, xsize, 0, 60, ysize-23, 60, ysize-3);
	boxfill8(vram, xsize, 0, 2, ysize-3, 60, ysize-3);
	boxfill8(vram, xsize, 15, xsize-47, ysize-24, xsize-4, ysize-24);
	boxfill8(vram, xsize, 15, xsize-47, ysize-23, xsize-47, ysize-4);
	boxfill8(vram, xsize, 0, xsize-47, ysize-3, xsize-4, ysize-3);
	boxfill8(vram, xsize, 0, xsize-3, ysize-24, xsize-3, ysize-3);
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d;
	for (i=0;i<16;i++){
		p=vram+(y+i)*xsize +x;
		d=font[i];
		if((d & 0x80)!=0){p[0]=c;}
		if((d & 0x40)!=0){p[1]=c;}
		if((d & 0x20)!=0){p[2]=c;}
		if((d & 0x10)!=0){p[3]=c;}
		if((d & 0x08)!=0){p[4]=c;}
		if((d & 0x04)!=0){p[5]=c;}
		if((d & 0x02)!=0){p[6]=c;}
		if((d & 0x01)!=0){p[7]=c;}
	}
	return;
}
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4094];
	for(;*s!=0x00;s++){
		putfont8(vram, xsize, x, y, 0, hankaku+ *s*16);
		x+=8;
	}
	return;
}


void init_mouse_cursor8(char *mouse, char bc) //准备鼠标指针
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x,y;
	for(y=0;y<16;y++){
		for(x=0;x<16;x++){
			if(cursor[y][x]=='O'){
				*(mouse+y*16+x)=7;
			}
			if(cursor[y][x]=='.'){
				*(mouse+y*16+x)=bc;
			}
			if(cursor[y][x]=='*'){
				*(mouse+y*16+x)=0;
			}
		}
	}
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize) //将鼠标图形缓存写入到vram中
{// *vram 是ram的地址， vxsize是scrnx pxsize、pysize是要显示的图形的size px0和py0是图形的起点位置 *buf是图像缓存的地址，bxsize是一行的像素数
	int x,y;
	for(y=0;y<pysize;y++){
		for(x=0;x<pxsize;x++){
			*(vram+py0*vxsize+px0+y*vxsize+x)=*(buf+y*bxsize+x);
		}
	}
	return;
}



void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt=(struct SEGMENT_DESCRIPTOR*) 0x00270000;
	struct GATE_DESCRIPTOR *idt=(struct GATE_DESCRIPTOR*) 0x0026f800;
	int i;
	//GDT的初始化
	for(i=0;i<8192;i++){
		set_segmdesc(gdt+i,0,0,0);
	}//因为GDT[0]是空描述符，必须保持为0。
	set_segmdesc(gdt +1 , 0xffffffff, 0x00000000, 0x4092);
	set_segmdesc(gdt +2 , 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);
	//IDT的初始化
	for(i=0;i<256;i++){
		set_gatedesc(idt+i,0,0,0);
	}
	load_idtr(0x7ff, 0x0026f800);

}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{// limit要用除法所以最好定义无符号数，base 和ar实际上也定义无符号数更好 作者这里ar的有效位为16bit，（4bit flags+4bit 0+ 8bit权限）
	if(limit > 0xffff){
		ar |= 0x8000; // G_bit=1 则颗粒度为4kb 作者这里bit15为 G_bit位
		limit /=0x1000;
	}
	sd->limit_low=limit & 0xffff;
	sd->base_low =base & 0xffff;
	sd->base_mid =(base>>16)&0xff;
	sd->access_right=ar & 0xff;
	sd->limit_high =((limit >> 16) & 0x0f) | ((ar>>8) & 0xf0);
	sd->base_high =(base >>24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar>>8) & 0xff;
	gd->access_right = ar&0xff;
	gd->offset_high=(offset>>16) & 0xff;
}

