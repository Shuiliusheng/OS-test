void api_putchar(int c);
void api_return();
void HariMain(void)
{
	int i=100;
	for(i=0;i<100;i++){
		api_putchar('a');
	}
	for(i=0;i<100;i++){
		api_putchar('b');
	}
	api_return();
}