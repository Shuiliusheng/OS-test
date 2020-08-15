#include"bootpack.h"

#define SHEET_USED 1
#define SHEET_FREE 0


sSHTCTL *shtctl_init(struct MEM_MAN *man, uchar *vram, int xsize, int ysize)
{
	sSHTCTL *shtctl;
	shtctl=(sSHTCTL *)mm_alloc(man, sizeof(sSHTCTL));
	if(shtctl==0)
		return 0;
	
	shtctl->vram = vram;
	shtctl->vxsize = xsize;
	shtctl->vysize = ysize;
	shtctl->top = -1;//没有图层要显示
	
	int i=0;
	for(i=0;i<MAX_SHEETS;i++){
		shtctl->sheets0[i].flags = SHEET_FREE;//未使用
	}
	return shtctl;
}

sSHEET *sheet_alloc(sSHTCTL *ctl)
{
	int i=0;
	sSHEET *sheet;
	for(i=0;i<MAX_SHEETS;i++)
	{
		if(ctl->sheets0[i].flags == SHEET_FREE){
			sheet = &(ctl->sheets0[i]);
			sheet->flags = SHEET_USED;
			sheet->height = -1;	//暂不显示
			return sheet;
		}
	}
	return 0;
}

void sheet_init(sSHEET *sht, uchar *buf, int bxsize, int bysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = bxsize;
	sht->bysize = bysize;
	sht->col_inv = col_inv;
	sht->vx0 = 0;
	sht->vy0 = 0;
	return ;
}

//将某个图层的显示层级进行更改，height为更改之后的层级
void sheet_updown(sSHTCTL *ctl, sSHEET *sht, int height)
{
	//最顶层
	if(height > ctl->top+1){
		height = ctl->top+1;
	}
	
	//不显示
	if(height < -1){
		height = -1;
	}
	
	int old_height = sht->height;
	int h=0;
	sht->height = height;
	
	//图层降低
	if(old_height > height){
		//原本是显示的图层
		if(height>=0){
			
			for(h=old_height;h>height;h--){
				ctl->sheets[h]=ctl->sheets[h-1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[h]=sht;
		}
		else{//不再显示
			//要隐藏的不是顶部的图层
			if(ctl->top > old_height){
				for(h=old_height;h<ctl->top;h++)
				{
					ctl->sheets[h]=ctl->sheets[h+1];
					ctl->sheets[h]->height=h;
				}
			}
			ctl->top--;
		}
		sheet_refresh(ctl);//刷新显示
	}
	//图层上升
	else if(old_height < height){
		//原本已经显示
		if(old_height>=0){
			for(h=old_height;h<height;h++){
				ctl->sheets[h]=ctl->sheets[h+1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[h]=sht;
		}
		else{
			for(h=ctl->top+1;h>height;h--){
				ctl->sheets[h]=ctl->sheets[h-1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[h]=sht;
			ctl->top++;
		}
		sheet_refresh(ctl);
	}
	
	char temp[100];
	sprintf(temp,"ctl->top:%d,%d,%d\n",ctl->top,height,old_height);
	show_string(temp,h+8);
	
	return ;
}

void sheet_refresh(sSHTCTL *ctl)
{
	uchar *vram = ctl->vram;
	uchar *buf,color;
	sSHEET *sht;
	int bx,by;
	int vx,vy;
	int h=0;
	for(h=0;h<=ctl->top;h++){
		buf = ctl->sheets[h]->buf;
		sht = ctl->sheets[h];
		for(by=0;by<sht->bysize;by++)
		{
			vy=sht->vy0+by;
			for(bx=0;bx<sht->bxsize;bx++){
				vx=sht->vx0+bx;
				color=buf[by*sht->bxsize+bx];
				
				if(color!=sht->col_inv){//不是透明色
					vram[vy*ctl->vxsize+vx]=color;
				}
			}
		}
		
		//char temp[100];
		//sprintf(temp,"%d:%d,%d,%d,%d\n",h,sht->bysize,sht->bxsize,sht->vx0,sht->vy0);
		//show_string(temp,h+8);
	}
}

//图层的平移
void sheet_slide(sSHTCTL *ctl, sSHEET *sht, int vx0, int vy0)
{
	sht->vx0=vx0;
	sht->vy0=vy0;
	if(sht->height>=0){
		sheet_refresh(ctl);
	}
	return ;
}

void sheet_free(sSHTCTL *ctl, sSHEET *sht)
{
	if(sht->height>=0){
		sheet_updown(ctl, sht, -1);
	}
	sht->flags = SHEET_FREE;
	return ;
}








