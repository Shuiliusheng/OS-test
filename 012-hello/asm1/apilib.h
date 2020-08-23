
void api_return();

//memory
void api_initmalloc();
char *api_malloc(unsigned int size);
void api_free(unsigned int addr);
unsigned int api_freesize();

//file
int api_file_open(char *filename, int flag);
void api_file_close(int handler);
int api_file_read(int handler, char *buf, int size);
int api_file_write(int handler, char *buf, int size);
int api_file_eof(int handler);
int api_file_seek(int handler, int offset, int flag);
int api_file_tell(int handler);

//input
char api_getchar(int flag);//0:not sleep; 1:sleep
int api_gets(char *str, int len);

//output
void api_puts(char *str);
void api_putchar(int c);

//timer
int api_allocTimer(void);
void api_initTimer(int timer, int value);
void api_setTimer(int timer, int msec);
void api_freeTimer(int timer);

//thread
unsigned int api_thread_create(int func_addr, int stack_addr);
void api_thread_run(unsigned int thread_id);
void api_thread_sleep(unsigned int thread_id);
void api_thread_delete(unsigned int thread_id);
int api_thread_isover(unsigned int thread_id);
