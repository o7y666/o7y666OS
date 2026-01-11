#include<stdio.h>
#include"bootpack.h"


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

