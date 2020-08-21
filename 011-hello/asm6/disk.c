#include"bootpack.h"

#define DISKSIZE 	8*1024*1024
struct DISKIINFO diskinfo;

int init_diskinfo()
{
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	diskinfo.addr = (uint) mm_alloc(man, DISKSIZE);
	if(diskinfo.addr == 0){
		diskinfo.sec_num =0 ;
		diskinfo.free_sec =0 ;
		return -1;//未分配成功
	}
	
	diskinfo.sec_num = DISKSIZE/512 ;
	uint tagsize = diskinfo.sec_num*4;
	tagsize = (tagsize%512==0)?tagsize/512:tagsize/512+1;
	
	//初始化tag，空磁盘应有的操作
	uint i=0;
	uint *temp = (uint *) mm_alloc(man, tagsize * 512);
	for(i=0;i<diskinfo.sec_num;i++){
		temp[i]=SECTOR_FREE;
	}
	for(i=0;i<tagsize;i++){
		temp[i]=SECTOR_INVALID;
	}
	writeSector(0, tagsize, (char *)temp, tagsize*512);
	mm_free(man,temp);
	
	//获取tag
	diskinfo.tag = (uint *)mm_alloc(man, tagsize * 512);
	readSector(0, tagsize, (char *)diskinfo.tag, tagsize*512);
	
	diskinfo.free_sec = diskinfo.sec_num - tagsize;
	diskinfo.tagsize = tagsize;
	return 1;//init success
}

void free_sector(uint sec)
{
	diskinfo.tag[sec]=SECTOR_FREE;
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	char *block = (char *)mm_alloc(man,512);
	int i=0;
	for(i=0;i<512;i++){
		block[i]=0;
	}
	writeSector(sec,1,block,512);
	mm_free(man, block);
}

void free_filesector(uint start_sec)
{
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	char *block = (char *)mm_alloc(man,512);
	int i=0;
	for(i=0;i<512;i++){
		block[i]=0;
	}
	uint temp;
	while(1){
		temp = diskinfo.tag[start_sec];
		diskinfo.tag[start_sec]=SECTOR_FREE;
		writeSector(start_sec,1,block,512);
		if(temp == SECTOR_FREE || temp == SECTOR_INVALID){
			break;
		}
	}
	mm_free(man, block);
}

void write_tag2disk()
{
	writeSector(0, diskinfo.tagsize, (char *)diskinfo.tag, diskinfo.tagsize*512);
}

uint find_free_sector()
{
	uint i=0;
	for(i=0;i<diskinfo.sec_num;i++){
		if(diskinfo.tag[i]==SECTOR_FREE){
			diskinfo.tag[i]=SECTOR_INVALID;
			return i;
		}
	}
	return SECTOR_INVALID;
}

uint used_sector_num()
{
	uint i=0;
	uint t=0;
	for(i=0;i<diskinfo.sec_num;i++){
		if(diskinfo.tag[i]!=SECTOR_FREE){
			t++;
		}
	}
	return t;
}

uint find_sector(uint start_sec, uint number)
{
	uint i=0;
	for(i=0;i<number;i++){
		if(diskinfo.tag[start_sec]!=SECTOR_INVALID 
			|| diskinfo.tag[start_sec]!=SECTOR_FREE){
			start_sec = diskinfo.tag[start_sec];
		}
		else{
			return SECTOR_INVALID;
		}
	}
	return start_sec;
}

uint alloc_sector(uint start_sec, uint number)
{
	uint i=0;
	uint sec=0;
	for(i=0;i<number;i++){
		if(diskinfo.tag[start_sec]==SECTOR_INVALID){
			sec = find_free_sector();	//寻找未被用的块
			if(sec == SECTOR_INVALID){	//not find
				return SECTOR_INVALID;
			}
			diskinfo.tag[start_sec] = sec;
			diskinfo.tag[sec]=SECTOR_INVALID;	//表示分配的块被占用了
		}
		start_sec = diskinfo.tag[start_sec];
	}
	return start_sec;
}

//pos从0开始
int writeDisk(uint start_sec, uint pos, char *buf, uint bufsize)
{
	int sec_num = pos/512;
	int first_sec = start_sec;
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	char *block = (char *)mm_alloc(man,512);
	uint index = pos%512, size=512-index;
	
	if(bufsize>0&&diskinfo.tag[start_sec]==SECTOR_FREE){
		diskinfo.tag[start_sec]=SECTOR_INVALID;
	}
	
	while(bufsize>0){
		start_sec = alloc_sector(start_sec, sec_num);
		if(start_sec == SECTOR_INVALID){
			write_tag2disk();
			return -1;	
		}
		readSector(start_sec, 1, block, 512);
		index = pos%512, size=512-index;
		if(bufsize <= size){
			mm_copy(&block[index],buf,bufsize);
			int t=writeSector(start_sec,1,block,512);
			bufsize = 0;
		}
		else{
			mm_copy(&block[index],buf,size);
			writeSector(start_sec,1,block,512);
			bufsize -= size;
			pos += size;
			buf += size;
		}
		sec_num=1;
	}
	mm_free(man, block);
	write_tag2disk();
	return 1;
	//return writeSector(first_sec, pos, buf, bufsize);
}

int readDisk(uint start_sec, uint pos, char *buf, uint bufsize)
{
	int sec_num = pos/512;
	int first_sec = start_sec;
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	char *block = (char *)mm_alloc(man,512);
	uint index = pos%512, size=512-index;
	
	while(bufsize>0){
		start_sec = find_sector(start_sec, sec_num);
		if(start_sec == SECTOR_INVALID){
			return -1; //超出了文件大小
		}
		readSector(start_sec, 1, block, 512);
		index = pos%512, size=512-index;
		if(bufsize <= size){
			mm_copy(buf,&block[index],bufsize);
			bufsize = 0;
		}
		else{
			mm_copy(buf,&block[index],size);
			bufsize -= size;
			pos += size;
			buf += size;
		}
		sec_num=1;
	}
	
	mm_free(man, block);
	return 1;
	//return writeSector(first_sec, pos, buf, bufsize);
}


//sector 从0开始
int readSector(uint sector, int sec_num, char *buf, uint bufsize)
{
	if(sector+sec_num >= diskinfo.sec_num){
		return -2;//overflow read
	}
	char *data = (char*)(diskinfo.addr + sector*512);
	uint i=0;
	uint limit = 512*sec_num;
	for(i=0;i<limit&&i<bufsize;i++){
		buf[i]=data[i];
	}
	return 1;	//read success
}

//sector 从0开始
int writeSector(uint sector, int sec_num, char *buf, uint bufsize)
{
	if(sector+sec_num >= diskinfo.sec_num){
		return -2;//overflow read
	}
	char *data = (char*)(diskinfo.addr + sector*512);
	uint i=0;
	uint limit = 512*sec_num;
	for(i=0;i<limit&&i<bufsize;i++){
		data[i]=buf[i];
	}
	return 1;	//read success
}