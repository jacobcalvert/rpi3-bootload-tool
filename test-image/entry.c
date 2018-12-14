#include "uart.h"
void entry()
{
	uart_putc(0x00);
	uart_puts("Hello, world!");
	while(1)
	{	
		uart_putc(uart_getc());
	}

}
