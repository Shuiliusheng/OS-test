#include "bootpack.h"



void hd_read(uint lba, int secs, char *buf)
{
	int cyln = lba / (2 * 18);
	int mo_val = (lba % (2 * 18));
	int head = mo_val / 18;
	int sect = mo_val % 18 + 1;

	while ((io_in8(0x1F7) & 0xC0) != 0x40);

	io_out8(0x1F2,secs);
	io_out8(0x1F3,sect);
	io_out8(0x1F4,cyln);
	io_out8(0x1F5, cyln >> 8);
	io_out8(0x1F6, head | 0xA0);

	io_out8(0x1F7, 0x20);
	while (!(io_in8(0x1F7) & 0x08));
	
	int i=0;
	uint word=123;
    for (i = 0; i < 5; i += 2){
		word = io_in16(0x1f0);
		buf[i] = word & 0xff;
		buf[i+1] = (word >> 8) & 0xff;
    }
}

