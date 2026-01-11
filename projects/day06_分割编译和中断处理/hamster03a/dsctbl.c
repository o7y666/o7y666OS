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

