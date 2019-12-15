#include <printer.h>
#include <system.h>

//def: status
#define STAT_NOT_INSTALLED    0
#define STAT_READY            1
#define STAT_ILLEGAL_CMD      2
#define STAT_BUSY             3
#define STAT_PRINT_ERR        4

//def: cmd
#define CMD_RESET    0
#define CMD_ACK      1
#define CMD_PRINT    2

static dtpreg_t* printer0 = (dtpreg_t*) DEV_REG_ADDR(IL_PRINTER, 0);

void printer_puts(const char* s) {
	char c;
  
	while((c = *s++)){
		printer_putchar(c);
	}
}

int printer_putchar(const char c){

	if(STAT_READY != printer0->status) return -1;

	printer0->data0 = c;
	printer0->command = CMD_PRINT;
	
	while(STAT_BUSY == printer0->status);

	printer0->command = CMD_ACK;
	return 0;
}

