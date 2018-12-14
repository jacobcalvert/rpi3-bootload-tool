#include "uart.h"

extern unsigned long __end;

void entry()
{
	uart_puts("Made it to low addresses\r\n");
	while(1);
}

