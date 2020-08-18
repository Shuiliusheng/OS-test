#include"bootpack.h"

sFILEINFO *find_file(sFILEINFO *fileinfo, char filename[], int subfind)
{
	int i=0,j=0,place=0;
	char str[30];
	
	int len=strlen(filename), t='A'-'a';
	for(i=0;i<len;i++){
		if(filename[i]>='a'&&filename[i]<='z'){
			filename[i]=filename[i]+t;
		}
	}
	
	for(i=0;i<224;i++){
		if(fileinfo[i].name[0]==0x00){
			//表示这一段不包括文件信息
			break;
		}
		if(fileinfo[i].name[0]!=0xe5){
			//0xe5表明该文件已被删除
			//0x10: 目录；0x08:非文件信息
			place=0;
			if( (fileinfo[i].type&0x18) == 0){
				for (j = 0; j < 8; j++) {
					if(fileinfo[i].name[j]!=' '){
						str[place] = fileinfo[i].name[j];
						place++;
					}
				}
				str[place] = '\0';
				if(subfind==1){
					show_string(str,28);
					if(strcmp_sub(filename,str)==0){
						return &fileinfo[i];
					}
				}
				else{
					str[place]='.';
					place++;
					for (j = 0; j < 3; j++) {
						if(fileinfo[i].ext[j]!=' '){
							str[place] = fileinfo[i].ext[j];
							place++;
						}
					}
					str[place] = '\0';
					
					//show_string(str,26+i);
					if(strcmp(filename,str)==0){
						return &fileinfo[i];
					}
				}
			}
		}
	}
	return 0;
}

void file_readfat(uint *fat, uchar *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

void file_readdata(uint *fat, char *buf, uint size, int clustno)
{
	char *img = (char*)(ADR_DISKIMG + 0x003e00);
	int i=0,j=0,place=0;
	while(size>=512){
		for(i=0;i<512;i++){
			buf[place]=img[clustno*512+i];
			place++;
		}
		clustno=fat[clustno];
		size-=512;
	}
	for(i=0;i<size;i++){
		buf[place]=img[clustno*512+i];
		place++;
	}
}