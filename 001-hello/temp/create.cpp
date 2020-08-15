#include<iostream>
#include<stdio.h>
using namespace std;

int main()
{
	FILE *p=fopen("helloos.img","wb");

	char c=0;
	for(int i=0;i<1440*1024;i++)
		fwrite(&c,1,1,p);
	fclose(p);
	return 0;
}