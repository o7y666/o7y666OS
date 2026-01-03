// naskfunc 中用汇编语言写的函数
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

//本文件中定义的c语言函数
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);


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

void HariMain(void)
{
	int i; 
	char *p;
	init_palette();//设定调色板
	p=(char *) 0xa0000;

	boxfill8(p, 320, 2, 20, 20, 120, 120);
	boxfill8(p, 320, 3, 30, 30, 140, 140);
	boxfill8(p, 320, 4, 40, 40, 160, 160);

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
		for(x=x0;x<x1;x++)
			vram[x+y*xsize]=c;
	}
	return;
}