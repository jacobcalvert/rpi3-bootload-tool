#include <stdint.h>
#include <stddef.h>

#include "uart.h"

#define MMIO_BASE       0x3F000000

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))


static int uart_initialized = 0;

#define MBOX_REQUEST    0

/* channels */
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_SETCLKRATE     0x38002
#define MBOX_TAG_LAST           0

volatile unsigned int  __attribute__((aligned(16))) mbox[36];

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch)
{
    unsigned int r;
    /* wait until we can write to the mailbox */
    while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        while(*MBOX_STATUS & MBOX_EMPTY);
        r=*MBOX_READ;
        /* is it a response to our message? */
        if((unsigned char)(r&0xF)==ch && (r&~0xF)==(unsigned int)((unsigned long)&mbox))
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}
static void uart_init()
{
	 register unsigned int r;

	    /* initialize UART */
	    *UART0_CR = 0;         // turn off UART0

	    /* set up clock for consistent divisor values */
	    mbox[0] = 8*4;
	    mbox[1] = MBOX_REQUEST;
	    mbox[2] = MBOX_TAG_SETCLKRATE; // set clock rate
	    mbox[3] = 12;
	    mbox[4] = 8;
	    mbox[5] = 2;           // UART clock
	    mbox[6] = 4000000;     // 4Mhz
	    mbox[7] = MBOX_TAG_LAST;
	    mbox_call(MBOX_CH_PROP);

	    /* map UART0 to GPIO pins */
	    r=*GPFSEL1;
	    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
	    r|=(4<<12)|(4<<15);    // alt0
	    *GPFSEL1 = r;
	    *GPPUD = 0;            // enable pins 14 and 15
	    r=150; while(r--);
	    *GPPUDCLK0 = (1<<14)|(1<<15);
	    r=150; while(r--);
	    *GPPUDCLK0 = 0;        // flush GPIO setup

	    *UART0_ICR = 0x7FF;    // clear interrupts
	    *UART0_IBRD = 2;       // 115200 baud
	    *UART0_FBRD = 0xB;
	    *UART0_LCRH = 0b11<<5; // 8n1
	    *UART0_CR = 0x301;     // enable Tx, Rx, FIFO
uart_initialized =1;
}

void uart_putc(unsigned char c)
{
	if(!uart_initialized)uart_init();
	 while(*UART0_FR&0x20);
	    /* write the character to the buffer */
	    *UART0_DR=c;
}

char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    while(*UART0_FR&0x10);
    /* read it and return */
    r=(char)(*UART0_DR);
    /* convert carrige return to newline */
    return r;
}
void uart_puts(char* str)
{
	
	for (size_t i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
}
