#include "uart.h"

extern unsigned long __end;

#define MAX_CHUNKSIZE		2048
#define BL_SESSION_START		0x93
#define BL_SESSION_END			0x39


#define RC_OK(b)				(b & 0x7F)
#define RC_ERROR(b)				(b | 0x80)

#define RC_OK_GENERIC			RC_OK(0)
#define RC_ERROR_GENERIC		RC_ERROR(0)
#define RC_ERROR_CHUNK_TOO_BIG	RC_ERROR(1)
#define RC_ERROR_CS_BAD			RC_ERROR(2)
#define RC_ERROR_UNKNOWN_CMD	RC_ERROR(3)


#define WAIT_FOR_BYTE(b)		while(uart_getc() != (b))

char CHUNK_BUFFER[MAX_CHUNKSIZE];

typedef enum bootloader_state
{
	STATE_IDLE,
	STATE_GETCMD,
	STATE_CMDFINISH

}bootload_state_t;

typedef char (*command_handler)(void);
typedef void (*fptr)(void);

char cmd_echo(void);
char cmd_setup_transfer(void);
char cmd_write_chunk(void);
char cmd_finalize_transfer(void);
char cmd_jump_addr(void);

static const command_handler COMMANDS[] = {
	[0] = cmd_echo,
	[1] = cmd_setup_transfer,
	[2] = cmd_write_chunk,
	[3] = cmd_finalize_transfer,
	[4] = cmd_jump_addr, 
	(command_handler)0

};

#define N_COMMANDS	sizeof(COMMANDS)/sizeof(command_handler)


static unsigned int start_address = 0;
static unsigned int current_offset = 0;
static unsigned int total_size = 0;
static char total_checksum = 0;


void relocate_bootloader(void);
void high_entry(void);
void entry(void)
{
	/* eventually I want to relocate the BL to the end of RAM so I can bootload at 0 */
	/*relocate_bootloader();*/
	high_entry();
}

void high_entry(void)
{
	bootload_state_t state = STATE_IDLE;
	char rc;
	uart_putc(0x00);
	while(1)
	{
		switch(state)
		{
			case STATE_IDLE:
			{
				WAIT_FOR_BYTE(BL_SESSION_START);
				state = STATE_GETCMD;
				break;
			}

			case STATE_GETCMD:
			{
				char cmd = uart_getc();
				if(cmd < N_COMMANDS)
				{
					rc = COMMANDS[(unsigned int)cmd]();
					
				}
				else
				{
					rc = RC_ERROR_UNKNOWN_CMD;
				}				
				state = STATE_CMDFINISH;
				break;
			
			}
			case STATE_CMDFINISH:
			{
				uart_putc(rc);
				uart_putc(BL_SESSION_END);
				state = STATE_IDLE;
				break;
				
			}
		}
	}
}
unsigned int get_int(void)
{
	unsigned int len = 0;
	len |= (unsigned int)uart_getc();
	len |=(unsigned int)uart_getc()<<8;
	len |=(unsigned int)uart_getc()<<16;
	len |=(unsigned int)uart_getc()<<24;
	return len;
}

char checksum(char *buffer,unsigned int size)
{
	char result = 0;
	unsigned int idx = 0;
	while(idx < size)
	{
		result += buffer[idx++];
	}
	return result;
}



char cmd_echo(void)
{
	unsigned int len = get_int();
	while(len--)
	{
		uart_putc(uart_getc());
	}
	return RC_OK_GENERIC;
}

char cmd_setup_transfer(void)
{
	start_address = get_int();
	total_size = get_int();
	total_checksum = (unsigned char) uart_getc();
	return RC_OK_GENERIC;
}

void memcpy(void *dst, void *src, unsigned int n)
{
	char *dPtr = (char*) dst;
	char *sPtr = (char*) src;
	unsigned int i = 0;
	while(i < n)
	{
		dPtr[i] = sPtr[i];
		++i;
	}
} 

char cmd_write_chunk(void)
{
	unsigned int idx = 0;
	unsigned int chunk_size = get_int();
	char cs;
	char *dptr = (char*)(start_address + current_offset);
	if(chunk_size > MAX_CHUNKSIZE)
	{
		return RC_ERROR_CHUNK_TOO_BIG;
	}
	while(idx < chunk_size)
	{
		CHUNK_BUFFER[idx++] = uart_getc();
	}	
	
	cs = uart_getc();
	
	if(cs != checksum(CHUNK_BUFFER, chunk_size))
	{
		return RC_ERROR_CS_BAD;
	}
	memcpy(dptr, CHUNK_BUFFER, chunk_size);
	current_offset += chunk_size;
	
	
	return RC_OK_GENERIC;
}

char cmd_finalize_transfer(void)
{

	char *start = (char *) start_address;
	char cs = checksum(start, total_size);
	if(cs == total_checksum)
	{
		return RC_OK_GENERIC;
	}
	return RC_ERROR_CS_BAD;

}

char cmd_jump_addr(void)
{
	unsigned int jump_addr = get_int();
	fptr fcn = (fptr) jump_addr;
	fcn();
	return RC_ERROR_GENERIC;
}


void relocate_bootloader(void)
{
	unsigned int end = &__end;
	unsigned int size = end;
	char *dst = (unsigned int)0x40000000 - size;
	char *src = (char*)0;
	unsigned int relocated_high_entry = (unsigned int) dst + (unsigned int) high_entry;
	fptr fcn = (fptr) relocated_high_entry;

	memcpy(dst, src, size);
	fcn();
	
	
	

}

