#include <stdio.h>
#include "bootpack.h"

#define SHEET_USE		1
// sheet control 的初始化，给sheet control分配一个内存空间，并设定每一个sheet都是可用的
struct SHTCTL *shtctl_init(struct MEMMAN *memman,unsigned char *vram,int xsize,int ysize)
{
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *)memman_alloc_4k(memman,sizeof(struct SHTCTL));
    if(ctl==0){
        goto err;
    }
    ctl->map=(unsigned char *)memman_alloc_4k(memman,xsize*ysize);
    ctl->vram=vram;
    ctl->xsize=xsize;
    ctl->ysize=ysize;
    ctl->top=-1;//none SHEET
    for(i=0;i<MAX_SHEETS;i++){
        ctl->sheets0[i].flags=0; // every sheet is unused 目前每个sheet都能用，但是还没被使用上。
        ctl->sheets0[i].ctl=ctl;//新建立的图层的管理器是ctl，标明当前图层的管理者是ctl
    }
    err:
        return ctl;

}
//为新建立的sheet分配内存空间
struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
    struct SHEET *sht;
    int i;
    for(i=0;i<MAX_SHEETS;i++){
        if(ctl->sheets0[i].flags==0){
            sht=&ctl->sheets0[i];
            sht->flags=SHEET_USE;
            sht->height=-1;//暂时先隐藏起来
            return sht;
        }
    }
    return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
    return;
}

void sheet_updown(struct SHEET *sht,int height)
{
    struct SHTCTL *ctl=sht->ctl;
    int h;
    int old_h=sht->height;
    //修正指定高度，以防超过了当前图层数量，比如现在一共就10个图层，要指定一个图层的高度是20时，会约束成最高的10
    if(height>ctl->top+1){
        height=ctl->top+1;
    }
    if(height<-1){
        height=-1;
    }
    sht->height=height;
    // 对 sheets【】进行重新排列
    if(old_h>height){// 图层要往下调整
        if(height>=0){ //height 往下的顺序不用变，height以上的都要移一格
            for(h=old_h;h>height;h--){
                ctl->sheets[h]=ctl->sheets[h-1];
                ctl->sheets[h]->height=h;
            }
            ctl->sheets[height]=sht;
            sheet_refreshmap(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1);
            sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->vx0+sht->bxsize,sht->vy0+sht->bysize,height+1,old_h);
        }
        else{//height=-1的话就是把这个sheet给隐藏了 old_h往下还是不用变，往上都要下降一格
           if(ctl->top>old_h){
                for(h=old_h;h<ctl->top;h++){
                    ctl->sheets[h]=ctl->sheets[h+1];
                    ctl->sheets[h]->height=h;
                }
           }
           ctl->top--;
           	sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old_h - 1);
        }
    } 
    else if (old_h<height){//图层要往上调整
        if(old_h>=0){
            for(h=old_h;h<height;h++){//把中间的都往下移
                ctl->sheets[h]=ctl->sheets[h+1];
                ctl->sheets[h]->height=h;
            }
            ctl->sheets[height]=sht;
        }
        else{//将隐藏的转换成可以显示的,height往上的都要移动一格
            for(h=ctl->top;h>=height;h--){
                ctl->sheets[h+1]=ctl->sheets[h];
                ctl->sheets[h+1]->height=h+1;
            }
            ctl->sheets[height]=sht;
            ctl->top++;
        }
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
    }
    return;
}

void sheet_refresh(struct SHEET *sht,int bx0,int by0,int bx1,int by1)
{
    //在图层内圈出（bx0，by0）到（bx1，by1）的矩形，只刷新这一部分 绝对位置（屏幕位置)靠sht的属性去定位
    if(sht->height>=0){
        sheet_refreshsub(sht->ctl, sht->vx0+bx0, sht->vy0+by0, sht->vx0+bx1, sht->vy0+by1,sht->height,sht->height);
    }
	return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	struct SHTCTL *ctl =sht->ctl;
    int old_vx0=sht->vx0,old_vy0=sht->vy0;
    sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { //图层都在显示就直接刷新图层即可
        sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize,0);
        sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize,sht->height);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize,0,sht->height-1);//原本图层的位置刷新一下 sht内的坐标已经改变
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize,sht->height,sht->height);//新位置的图层刷新一下
	}
	return;
}


void sheet_free(struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(sht, -1); //将图层高度设置为-1，隐藏起来
	}
	sht->flags = 0; //将图层设置为未使用
	return;
}

void sheet_refreshsub(struct SHTCTL *ctl,int vx0,int vy0,int vx1,int vy1,int h0,int h1)// vx0和vy0等是要刷新区域在屏幕上的绝对坐标
{
	 int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, *vram = ctl->vram,*map=ctl->map,sid;
	struct SHEET *sht;
    //修正刷新范围
    if(vx0<0) {vx0=0;}
    if(vy0<0) {vy0=0;}
    if(vx1>ctl->xsize) {vx1=ctl->xsize;}
    if(vy1>ctl->ysize) {vy1=ctl->ysize;}
	for (h = h0; h <= h1; h++) {//从下往上
		sht = ctl->sheets[h];
		buf = sht->buf;
        sid=sht-ctl->sheets0;
        /* 关键优化：计算刷新区域在图层内的对应区域 */
        bx0 = vx0 - sht->vx0;  // 屏幕坐标 → 图层坐标
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        /* 边界检查：确保不超出图层范围 */ //如果刷新区域就在图层范围里，这样转换坐标是没问题的，但是刷新区域只有部分在图层中则只能刷新在图层中的这部分
        if (bx0 < 0) { bx0 = 0; }           // 说明(1)：左边界裁剪
        if (by0 < 0) { by0 = 0; }           // 上边界裁剪
        if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }  // 说明(2)：右边界裁剪
        if (by1 > sht->bysize) { by1 = sht->bysize; }   // 下边界裁剪
        // 只遍历需要刷新的部分
        for (by = by0; by < by1; by++) {      // 只遍历有效行
            vy=sht->vy0+by;
            for (bx = bx0; bx < bx1; bx++) {   // 只遍历有效列
                vx = sht->vx0 + bx;
                if(map[vy*ctl->xsize+vx]==sid){
                    vram[vy*ctl->xsize+vx]=buf[by*sht->bxsize+bx];
                }
            }
        }
	}
	return;
}

//用图层号代替色号
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, sid, *map = ctl->map;
    struct SHEET *sht;
    if (vx0 < 0) { vx0 = 0; }
    if (vy0 < 0) { vy0 = 0; }
    if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
    if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
    for (h = h0; h <= ctl->top; h++) {
        sht = ctl->sheets[h];
        sid = sht - ctl->sheets0; /* 将进行了减法计算的地址作为图层号码使用，实际是sid = (sht的地址 - ctl->sheets0的地址) / sizeof(struct SHEET)*/
        buf = sht->buf;                                               //指针的减法得到的是地址之间的元素个数
        bx0 = vx0- sht->vx0;
        by0 = vy0- sht->vy0;
        bx1 = vx1- sht->vx0;
        by1 = vy1- sht->vy0;
        if (bx0 < 0) { bx0 = 0; }
        if (by0 < 0) { by0 = 0; }
        if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
        if (by1 > sht->bysize) { by1 = sht->bysize; }
        for (by = by0; by < by1; by++) {
            vy = sht->vy0 + by;
            for (bx = bx0; bx < bx1; bx++) {
                vx = sht->vx0 + bx;
                if (buf[by * sht->bxsize + bx] != sht->col_inv) {
                    map[vy * ctl->xsize + vx] = sid;
                }
            }
        }
    }
    return;
}