void api_return();

void HariMain(void)
{
	*((char *)0x00102600)=0;
	
	api_return();
}