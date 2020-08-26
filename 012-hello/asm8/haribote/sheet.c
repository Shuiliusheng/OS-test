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
	shtctl->map = (uchar *) mm_alloc(man, xsize * ysize);
	shtctl->top = -1;//没有图层要显示
	if(shtctl->map == 0){
		return shtctl;
	}
	
	int i=0;
	for(i=0;i<MAX_SHEETS;i++){
		shtctl->sheets0[i].flags = SHEET_FREE;//未使用
		shtctl->sheets0[i].ctl = shtctl;//未使用
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
	sht->task = 0;
	return ;
}

//将某个图层的显示层级进行更改，height为更改之后的层级
void sheet_updown(sSHEET *sht, int height)
{
	sSHTCTL *ctl=sht->ctl;
	
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
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height+1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height+1, old_height);
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
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old_height-1);
		}
		
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
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height,height);
	}
	
	return ;
}

void sheet_refreshmap(sSHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	uchar *map = ctl->map;
	uchar *buf,color;
	sSHEET *sht;
	int bx,by;
	int vx,vy;
	int sx,sy,ex,ey;
	int t1,t2,t3;
	int h=0;
	int sid;
	int *imap,sid4;
	char *cmap;
	int i=0;
	
	if(vx0<0) {vx0=0;}
	if(vy0<0) {vy0=0;}
	if(vx1>ctl->vxsize) {vx1=ctl->vxsize;}
	if(vy1>ctl->vysize) {vy1=ctl->vysize;}
	
	for(h=h0;h<=ctl->top;h++){
		buf = ctl->sheets[h]->buf;
		sht = ctl->sheets[h];
		
		sx=vx0-sht->vx0;
		sy=vy0-sht->vy0;
		ex=vx1-sht->vx0;
		ey=vy1-sht->vy0;
		
		if(sx<0){sx=0;}
		if(sy<0){sy=0;}
		
		if(ex>sht->bxsize) {ex=sht->bxsize;}
		if(ey>sht->bysize) {ey=sht->bysize;}
		
		sid = sht-ctl->sheets0;	//序号
		sid4 = sid | sid << 8 | sid << 16 | sid << 24;
		
		if (sht->col_inv == -1) {
			t1 = (sy+sht->vy0-1)*ctl->vxsize + sht->vx0 + sx;
			t2 = (ex-sx)/4;
			t3 = (ex-sx)%4;
			for(by=sy;by<ey;by++){
				t1 += ctl->vxsize;
				imap = (int *)&(map[t1]);
				for(i=0;i<t2;i++){
					imap[i] = sid4;
				}
				cmap = (char*)&imap[i];
				for(i=0;i<t3;i++){
					cmap[i] = sid;
				}
			}	
		}
		else{
			t2 = (sy + sht->vy0)*ctl->vxsize;
			t1 = sy*sht->bxsize;
			for(by=sy;by<ey;by++){
				for(bx=sx;bx<ex;bx++){
					vx=bx+sht->vx0;
					color=buf[t1+bx];
					if(color!=sht->col_inv){
						map[t2+vx]=sid;
					}
				}
				t2 += ctl->vxsize;
				t1 += sht->bxsize;
			}	
		}
	}
}

void sheet_refreshsub(sSHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	uchar *vram = ctl->vram;
	uchar *map  = ctl->map;
	uchar *buf,color;
	sSHEET *sht;
	int bx,by;
	int vx,vy;
	int sx,sy,ex,ey;
	int t1,t2,t3,t4;
	int h=0;
	int sid,sid4,i=0,j=0;
	int *ivram,*imap,*ibuf;
	char *cvram,*cmap, *cbuf;
	
	if(vx0<0) {vx0=0;}
	if(vy0<0) {vy0=0;}
	if(vx1>ctl->vxsize) {vx1=ctl->vxsize;}
	if(vy1>ctl->vysize) {vy1=ctl->vysize;}
	
	for(h=h0;h<=h1;h++){
		buf = ctl->sheets[h]->buf;
		sht = ctl->sheets[h];
		
		sx=vx0-sht->vx0;
		sy=vy0-sht->vy0;
		ex=vx1-sht->vx0;
		ey=vy1-sht->vy0;
		
		if(sx<0){sx=0;}
		if(sy<0){sy=0;}
		
		if(ex>sht->bxsize) {ex=sht->bxsize;}
		if(ey>sht->bysize) {ey=sht->bysize;}
		
		sid=sht-ctl->sheets0;
		sid4 = sid | sid << 8 | sid << 16 | sid << 24;
		
		t1 = (sy + sht->vy0 - 1)*ctl->vxsize + (sx + sht->vx0);
		t2 = (sy-1)*sht->bxsize+sx;
		t3 = (ex-sx)/4;
		t4 = (ex-sx)%4;
		for(by=sy;by<ey;by++){
			t1+=ctl->vxsize;
			t2+=sht->bxsize;
			imap = (int *)&(map[t1]);
			ivram = (int *)&(vram[t1]);
			ibuf = (int *)&buf[t2];
			
			for(i=0;i<t3;i++){
				if(imap[i] == sid4){
					ivram[i] = ibuf[i];
				}
				else{
					cmap = (char*)(&imap[i]);
					cvram = (char*)(&ivram[i]);
					cbuf = (char*)(&ibuf[i]);
					for(j=0;j<4;j++){
						if(cmap[j]==sid){
							cvram[j]=cbuf[j];
						}
					}
				}
			}
			
			cmap = (char*)(&imap[i]);
			cvram = (char*)(&ivram[i]);
			cbuf = (char*)(&ibuf[i]);
			for(j=0;j<t4;j++){
				if(cmap[j]==sid){
					cvram[j]=cbuf[j];
				}
			}
		}
		
	}
}



void sheet_refresh(sSHEET *sht, int bx0, int by0, int bx1, int by1)
{
	sSHTCTL *ctl=sht->ctl;
	if (sht->height >= 0) { 
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
	}
}

//图层的平移
void sheet_slide(sSHEET *sht, int vx0, int vy0)
{
	sSHTCTL *ctl=sht->ctl;
	int oldx,oldy;
	oldx=sht->vx0;
	oldy=sht->vy0;
	sht->vx0=vx0;
	sht->vy0=vy0;
	if(sht->height>=0){
		sheet_refreshmap(ctl,oldx,oldy,oldx+sht->bxsize,oldy+sht->bysize, 0);		
		sheet_refreshmap(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize, sht->height);
		sheet_refreshsub(ctl,oldx,oldy,oldx+sht->bxsize,oldy+sht->bysize, 0, sht->height-1);
		sheet_refreshsub(ctl,vx0,vy0,vx0+sht->bxsize,vy0+sht->bysize, sht->height,sht->height);
	}
}

void sheet_free(sSHEET *sht)
{
	if(sht->height>=0){
		sheet_updown(sht, -1);
	}
	sht->flags = SHEET_FREE;
	sht->task = 0;
	return ;
}








