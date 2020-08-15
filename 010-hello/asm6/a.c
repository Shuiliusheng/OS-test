void api_putchar(int c);
void api_return();
void api_expand(unsigned int size);
void HariMain(void)
{
	int i=100;
	for(i=0;i<100;i++){
		api_putchar('a');
	}
	api_expand(1024);
	for(i=0;i<100;i++){
		api_putchar('b');
	}
	api_return();
}