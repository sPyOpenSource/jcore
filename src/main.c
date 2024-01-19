#include "BCM2837.h"
#include <stdint.h>
 
void _start()
{
    uint32_t reg;
    uint32_t *mmio_base;
    char *board;
 
    /* read the system register */
#if __AARCH64__
    asm volatile ("mrs %x0, midr_el1" : "=r" (reg));
#else
    asm volatile ("mrc p15,0,%0,c0,c0,0" : "=r" (reg));
#endif
 
    /* get the PartNum, detect board and MMIO base address */
    switch ((reg >> 4) & 0xFFF) {
        case 0xB76: board = "Rpi1"; mmio_base = 0x20000000; break;
        case 0xC07: board = "Rpi2"; mmio_base = 0x3F000000; break;
        case 0xD03: board = "Rpi3"; mmio_base = 0x3F000000; break;
        case 0xD08: board = "Rpi4"; mmio_base = 0xFE000000; break;
        default:    board = "????"; mmio_base = 0x20000000; break;
    }
}

typedef unsigned int uint;

void write32(void *dst, uint val)
{
	uint* dst_u = (uint*)dst;
	*dst_u = val;

	return;
}

uint read32(void *src)
{
	uint* src_u = (uint*)src;
	return *src_u;
}

void uart_init(int raspi)
{
	mmio_init(raspi);
 
	// Disable UART0.
	mmio_write(UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.
 
	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	mmio_write(GPPUD, 0x00000000);
	delay(150);
 
	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
 
	// Write 0 to GPPUDCLK0 to make it take effect.
	mmio_write(GPPUDCLK0, 0x00000000);
 
	// Clear pending interrupts.
	mmio_write(UART0_ICR, 0x7FF);
 
	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// Baud = 115200.
 
	// For Raspi3 and 4 the UART_CLOCK is system-clock dependent by default.
	// Set it to 3Mhz so that we can consistently set the baud rate
	if (raspi >= 3) {
		// UART_CLOCK = 30000000;
		unsigned int r = (((unsigned int)(&mbox) & ~0xF) | 8);
		// wait until we can talk to the VC
		while ( mmio_read(MBOX_STATUS) & 0x80000000 ) { }
		// send our message to property channel and wait for the response
		mmio_write(MBOX_WRITE, r);
		while ( (mmio_read(MBOX_STATUS) & 0x40000000) || mmio_read(MBOX_READ) != r ) { }
	}
 
	// Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
	mmio_write(UART0_IBRD, 1);
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	mmio_write(UART0_FBRD, 40);
 
	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
	mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
 
	// Mask all interrupts.
	mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
	                       (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
 
	// Enable UART0, receive & transfer part of UART.
	mmio_write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}
 
void uart_putc(unsigned char c)
{
	// Wait for UART to become ready to transmit.
	while ( mmio_read(UART0_FR) & (1 << 5) ) { }
	mmio_write(UART0_DR, c);
}
 
unsigned char uart_getc()
{
    // Wait for UART to have received something.
    while ( mmio_read(UART0_FR) & (1 << 4) ) { }
    return mmio_read(UART0_DR);
}
 
void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i ++)
		uart_putc((unsigned char)str[i]);
}

void main(void)
{
	uint gpfsel2 = read32(BCM2837_GPFSEL2);
	gpfsel2 |= (1<<3); //turn pin 21 into an output.
	write32(BCM2837_GPFSEL2, gpfsel2);

	int i = 0;

	while(1)
	{
		//turn on pin 21
		write32(BCM2837_GPSET0, 1 << 21);

		//delay
		i = 0;
		while (i < 0x800000)
		{
			i++;
		}

		//turn off pin 21
		write32(BCM2837_GPCLR0, 1 << 21);

		//delay
		i = 0;
		while (i < 0x800000)
		{
			i++;
		}

	}
}
