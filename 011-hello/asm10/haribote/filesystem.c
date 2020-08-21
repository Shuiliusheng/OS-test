#include"bootpack.h"

extern struct DISKIINFO diskinfo;

struct FILEINFO_SECTOR{
	sFileInfoS fileinfos[16];
	struct FILEINFO_SECTOR *next;
	struct FILEINFO_SECTOR *end;
};
#define sFILEINFO_SECTOR struct FILEINFO_SECTOR
sFILEINFO_SECTOR *fileinfo_sec;

void init_filesystem_disk()
{
	int sec = diskinfo.tagsize;//tag之后的那一个sector
	sFileInfoS info;
	strcpy(info.filename,"root");
	info.type = 0xe0;	//used and dir
	info.sec = sec;
	info.size = 1;		//用于表示一共多少个file information
	info.ndir = 0;
	diskinfo.tag[sec] = SECTOR_INVALID;
	writeDisk(sec, 0, (char *)&info, 32);
}

int read_fileinfo()
{
	int sec = diskinfo.tagsize;
	sFileInfoS info1;
	info1.size=0;
	readSector(sec, 1, (char *)&info1, 32);
	if(info1.type != 0xe0 || strcmp(info1.filename,"root") != 0){
		return -1;//read failed
	}
	
	int info_num = info1.size;
	int i=0,num=0;
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	sFILEINFO_SECTOR *filetemp;
	for(i=0;i<info_num;i+=16){
		num = 16;
		if(info_num-i<16){
			num = info_num-i;
		}
		if(i==0){
			fileinfo_sec = (sFILEINFO_SECTOR *)mm_alloc(man, sizeof(sFILEINFO_SECTOR));
			fileinfo_sec->next = 0;
			fileinfo_sec->end = fileinfo_sec;
			readDisk(sec, i*32, (char *)(fileinfo_sec->fileinfos), 32*num);
		}
		else{
			filetemp = (sFILEINFO_SECTOR *)mm_alloc(man, sizeof(sFILEINFO_SECTOR));
			filetemp->next = 0;
			fileinfo_sec->end->next = filetemp;
			fileinfo_sec->end = filetemp;
			readDisk(sec, i*32, (char *)(filetemp->fileinfos), 32*num);
		}
	}
	return 1;//success;
}

sFileInfoS *index_fileinfo(int index)
{
	sFILEINFO_SECTOR *first = fileinfo_sec;
	int n = index/16;
	int i=0;
	for(i=0;i<n;i++){
		if(first->next == 0){
			return 0;
		}
		first = first->next;
	}
	return &first->fileinfos[index%16];
}

//创建新文件时，寻找一个free的
int alloc_fileinfo(int *addr)
{
	int num = fileinfo_sec->fileinfos[0].size;
	int i=0;
	sFileInfoS *info;
	for(i=1;i<num;i++){
		info = index_fileinfo(i);
		if(info->type == FILEINFO_FREE){//
			info->type = FILEINFO_ALLOC;
			return i;
		}
	}
	if(num%16==0){
		struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
		sFILEINFO_SECTOR *filetemp = (sFILEINFO_SECTOR *)mm_alloc(man, sizeof(sFILEINFO_SECTOR));
		filetemp->next=0;
		fileinfo_sec->end->next = filetemp;
		fileinfo_sec->end=filetemp;
	}
	info = index_fileinfo(num);
	*addr = (int)info;
	info->type = FILEINFO_ALLOC;
	fileinfo_sec->fileinfos[0].size++;
	return num;
}

//根据文件名找index
int find_fileinfo(char filename[], int dir, int *addr)
{
	int num = fileinfo_sec->fileinfos[0].size;
	int i=0;
	sFileInfoS *info;
	for(i=1;i<num;i++){
		info = index_fileinfo(i);
		if(info->type != FILEINFO_FREE && info->ndir == dir
			&& strcmp(filename,info->filename)==0)
		{
			*addr = (int)info;
			return i;
		}
	}
	return 0;
}

int used_fileinfo()
{
	int num = fileinfo_sec->fileinfos[0].size;
	int i=0;
	int t=0;
	sFileInfoS *info;
	for(i=0;i<num;i++){
		info = index_fileinfo(i);
		if(info->type != FILEINFO_FREE){
			t++;
		}
	}
	return t;
}

int show_dirinfo()
{
	int num = fileinfo_sec->fileinfos[0].size;
	int i=0;
	char str[20];
	sFileInfoS *info;
	for(i=0;i<num;i++){
		info = index_fileinfo(i);
		if(info->type == FILEINFO_DIR){
			sprintf(str,"dir:%s,%d,%d",info->filename,info->size, info->sec);
			show_string(str,1);
		}
	}
	return 0;
}


////////////////////////////////////////////////////////////////////

void fs_dir_delete(int ndir)
{
	sFileInfoS *info = index_fileinfo(ndir);
	info->type = FILEINFO_FREE;
	int n=0;
	int num = fileinfo_sec->fileinfos[0].size;
	for(n=1;n<num;n++){
		info = index_fileinfo(n);
		if(info->ndir != ndir){
			continue;
		}
		if( (info->type&0x0f) ==FILEINFO_FILE){
			free_filesector(info->sec);
			info->type = FILEINFO_FREE;
		}
		else if((info->type&0x0f)==FILEINFO_DIR){
			fs_dir_delete(n);
		}
	}
	
}

//删除文件，
int fs_delete(int currentDir, char filename[])
{
	int addr=0;
	int n=find_fileinfo(filename,currentDir, &addr);
	sFileInfoS *info=(sFileInfoS *)addr;
	if(n==0){
		return -1;//未找到文件
	}
	if((info->type&0xf0) == FILEINFO_WBUSY){
		return -2;//文件正在写
	}
	if((info->type&0xf0) == FILEINFO_RBUSY){
		return -3;//文件正在读
	}
	
	if( (info->type&0x0f) == FILEINFO_FILE){
		free_filesector(info->sec);
		info->type = FILEINFO_FREE;
	}
	else if( (info->type&0x0f) == FILEINFO_DIR){
		if(info->sec==0&&info->size==0){
			//sec: read, size:write busy number
			fs_dir_delete(n);
		}
		else{
			return -4;
		}
	}
	return 1;
}

//type file
void fs_type(sCONSOLEINFO *cons, int currentDir, char filename[])
{
	int addr=0;
	int n=find_fileinfo(filename, currentDir, &addr);
	sFileInfoS *info=(sFileInfoS *)addr;
	if(n==0){
		show_string_sheet(cons,"file is not exist!",1);
		return;
	}
	if((info->type&0x0f) == FILEINFO_DIR){
		show_string_sheet(cons,"this is dir!",1);
		return;
	}
	if((info->type&0xf0) == FILEINFO_WBUSY){
		show_string_sheet(cons,"file is being writed!",1);
		return ;
	}
	
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	
	sTASK *task = task_now();
	task->flag_allocRes = 1;
	char *buf = (char *)mm_alloc(man, info->size);
	sHandlerInfo *handler=allocHandler(task, man,(uint)buf, HANDLER_CONS);
	task->flag_allocRes = 0;
	
	readDisk(info->sec,0,buf,info->size);
		
	//显示
	char str[2];
	int i=0;
	str[1]='\0';
	for(i=0;i<info->size;i++){
		str[0]=buf[i];
		show_string_sheet(cons,str,0);
	}
	
	releaseHandlerSingle(task, handler);
	//mm_free(man,(int)buf);
	
	cons->cursorX=8;
	console_newline(cons);
	console_newline(cons);
}

void fs_showdir(sCONSOLEINFO *cons, int index)
{
	sFileInfoS *info = index_fileinfo(index);
	if(index==0){
		show_string_sheet(cons,"/root/",0);
		return ;
	}
	fs_showdir(cons,info->ndir);
	show_string_sheet(cons,info->filename,0);
	show_string_sheet(cons,"/",0);
	return;
}

//显示文件列表
void fs_showlist(sCONSOLEINFO *cons, int dir)
{
	int num = fileinfo_sec->fileinfos[0].size;
	int i=0;
	sFileInfoS *info;
	char str[20];
	for(i=1;i<num;i++){
		info = index_fileinfo(i);
		if(info->type != FILEINFO_FREE && info->ndir == dir)
		{
			if((info->type&0x0f)==FILEINFO_FILE){
				sprintf(str,"FILE %6dB  ",info->size);
			}
			else{
				sprintf(str,"DIR   ");
			}
			show_string_sheet(cons,str,0);
			//fs_showdir(cons,info->ndir);
			show_string_sheet(cons,info->filename,1);
		}
	}
}

//create dir
void fs_createDir(int currentDir, char dirname[])
{
	int addr=0;
	int n = alloc_fileinfo(&addr);
	sFileInfoS *info = (sFileInfoS *)addr;
	strcpy(info->filename,dirname);
	info->size = 0;
	info->ndir = currentDir;
	info->type = FILEINFO_DIR;
	info->sec = 0;
}

int fs_back_dir(int currentDir)
{
	sFileInfoS *info = index_fileinfo(currentDir);
	return info->ndir;
}

int fs_find_dir(int currentDir, char dirname[])
{
	int num = fileinfo_sec->fileinfos[0].size;
	int i=0;
	sFileInfoS *info;
	for(i=1;i<num;i++){
		info = index_fileinfo(i);
		if(info->type == FILEINFO_DIR && info->ndir == currentDir
			&&strcmp(dirname,info->filename)==0)
		{
			return i;
		}
	}
	return currentDir;
}

void fs_update_dirflag(int ndir, int flag, int type)
{
	if(ndir==0){
		return;
	}
	sFileInfoS *info = index_fileinfo(ndir);
	if(type == FILEHANDLER_R){
		info->size += flag;
	}
	else{
		info->sec += flag;
	}
	fs_update_dirflag(info->ndir,flag,type);
}

////////////////////////////////////////////////////////////////////

sFileHandler *file_open(char filename[], char flag,int ndir)
{	
	int i=0,n=0, addr=0;
	struct MEM_MAN *man = (struct MEM_MAN *) MEMMAN_ADDR;
	sFileInfoS *info;
	if(flag == 'r'){
		n=find_fileinfo(filename, ndir, &addr);
		info = (sFileInfoS *)addr;
		if(n==0){
			return 0;
		}
		if((info->type&0xf0) == FILEINFO_WBUSY){
			return 0;//文件正在写
		}
		//设置文件读标志
		info->type = info->type|FILEINFO_RBUSY;
		
		sFileHandler *handler;
		handler = (sFileHandler *)mm_alloc(man, sizeof(sFileHandler));
		handler->start_sec = info->sec;
		handler->pos = 0;
		handler->flag = FILEHANDLER_R;
		handler->index = n;
		
		fs_update_dirflag(ndir, 1, FILEHANDLER_R);
		//show_dirinfo();
		return handler;
	}
	else if(flag == 'w'){
		n=find_fileinfo(filename,ndir, &addr);
		if(n==0){
			n = alloc_fileinfo(&addr);
			info = (sFileInfoS *)addr;
			strcpy(info->filename,filename);
			info->size = 0;
			info->ndir = ndir;
			info->type = FILEINFO_FILE;
			info->sec = SECTOR_INVALID;
		}
		else{
			info = (sFileInfoS *)addr;
			if((info->type&0xf0) == FILEINFO_WBUSY){
				return 0;//文件正在写
			}
			free_filesector(info->sec);
			info->size = 0;
			info->ndir = ndir;
			info->type = FILEINFO_FILE;
			info->sec = SECTOR_INVALID;
		}
		//设置文件写标志
		info->type = info->type|FILEINFO_WBUSY;
		
		sFileHandler *handler;
		handler = (sFileHandler *)mm_alloc(man, sizeof(sFileHandler));
		handler->start_sec = info->sec;
		handler->pos = 0;
		handler->flag = FILEHANDLER_W;
		handler->index = n;
		fs_update_dirflag(ndir, 1, FILEHANDLER_W);
		//show_dirinfo();
		return handler;
	}
	else if(flag == 'a'){
		n=find_fileinfo(filename, ndir,&addr);
		if(n==0){
			n = alloc_fileinfo(&addr);
			info = (sFileInfoS *)addr;
			strcpy(info->filename,filename);
			info->size = 0;
			info->ndir = ndir;
			info->type = FILEINFO_FILE;
			info->sec = SECTOR_INVALID;
		}
		info = (sFileInfoS *)addr;
		if((info->type&0xf0) == FILEINFO_WBUSY){
			return 0;//文件正在写
		}
		//设置文件写标志
		info->type = info->type|FILEINFO_WBUSY;
		
		sFileHandler *handler;
		handler = (sFileHandler *)mm_alloc(man, sizeof(sFileHandler));
		handler->start_sec = info->sec;
		handler->pos = info->size;
		handler->flag = FILEHANDLER_A;
		handler->index = n;
		fs_update_dirflag(ndir, 1, FILEHANDLER_W);
		return handler;
	}
	else{
		return 0;
	}
}

void file_close(sFileHandler *handler)
{
	if(handler == 0 || handler->flag == FILEHANDLER_FREE){
		return ;
	}

	sFileInfoS *info = index_fileinfo(handler->index);
	info->type = info->type&0x0f;	//	清楚使用标识
	
	fs_update_dirflag(info->ndir, -1, handler->flag);
	//show_dirinfo();
	handler->flag = FILEHANDLER_FREE;
	update_fileinfo(handler->index);
	
	//handler = 0;
}

//判断是否文件末尾
int file_eof(sFileHandler *handler)
{
	if(handler == 0 || handler->flag == FILEHANDLER_FREE){
		return 1;
	}
	if(handler->flag!=FILEHANDLER_R){
		return 1;//不是写模式，不进行读取
	}
	
	int index = handler->index;
	sFileInfoS *info = index_fileinfo(index);
	if(info->size - handler->pos <=0){
		return 1;
	}
	
	return 0;
}

//读取文件数据
int file_read(sFileHandler *handler, char *buf, uint size)
{
	if(handler == 0 || handler->flag == FILEHANDLER_FREE){
		return 0;
	}
	
	if(handler->flag!=FILEHANDLER_R){
		return 0;//不是写模式，不进行读取
	}
	
	int index = handler->index;
	sFileInfoS *info = index_fileinfo(index);
	if(info->size - handler->pos <=0){
		return 0;
	}
	
	if(info->size - handler->pos <= size){
		size = info->size - handler->pos;
	}
	
	index = readDisk(handler->start_sec, handler->pos, buf, size);
	if(index == -1){
		return 0;
	}
	handler->pos += size;
	return size;
}

//向文件写入数据
int file_write(sFileHandler *handler, char *buf, uint size)
{
	if(handler == 0 || handler->flag == FILEHANDLER_FREE){
		return 0;
	}
	
	if(handler->flag!=FILEHANDLER_W&&handler->flag!=FILEHANDLER_A){
		return 0;	//不是可写的模式
	}
	
	int index = handler->index;
	
	sFileInfoS *info = index_fileinfo(index);
	if(size>0&&handler->start_sec==SECTOR_INVALID){
		handler->start_sec = find_free_sector();
		info->sec = handler->start_sec;
		update_fileinfo(index);
	}
	
	int t=writeDisk(handler->start_sec, handler->pos, buf, size);
	if(t==-1){
		return 0;
	}
	
	handler->pos=size+handler->pos;
	if(info->size < handler->pos){
		info->size = handler->pos;
		update_fileinfo(index);
	}
	
	return size;
}

//定位文件指针
int file_seek(sFileHandler *handler, int offset, uchar flag)
{
	if(handler == 0 || handler->flag == FILEHANDLER_FREE){
		return 0;
	}
	int index = handler->index;
	sFileInfoS *info = index_fileinfo(index);
	int size = info->size;
	if(flag == SEEK_SET){
		handler->pos = offset;
	}
	else if(flag == SEEK_CUR){
		handler->pos += offset;
	}
	else if(flag == SEEK_END){
		handler->pos = (size+offset);
	}
	if(handler->pos <0){
		handler->pos=0;
	}
	if(handler->flag == FILEHANDLER_R && handler->pos>size){
		handler->pos = size;
	}
	return handler->pos;
}

//返回文件指针位置
int file_tell(sFileHandler *handler)
{
	if(handler == 0 || handler->flag == FILEHANDLER_FREE){
		return 0;
	}
	return handler->pos;
}


///将fileinfo更新到磁盘中
void update_fileinfo(int n)
{
	if(n>=fileinfo_sec->fileinfos[0].size){
		return ;
	}
	
	sFILEINFO_SECTOR *first = fileinfo_sec;
	n = n/16;
	int i=0;
	for(i=0;i<n;i++){
		if(first->next == 0){
			return ;
		}
		first = first->next;
	}
	int sec = fileinfo_sec->fileinfos[0].sec;
	writeDisk(sec,n*512,(char*)(first->fileinfos), 512);
}
