#ifndef __IRQ_H
#define __IRQ_H

#ifndef ASSEMBLER

#ifdef APIC
#define NUM_IRQs 256
#else
#define NUM_IRQs 16
#endif

struct irqfunctions {
	const char *PICname;
	void (*ack) (unsigned int nr);
	void (*enable) (unsigned int nr);
	void (*disable) (unsigned int nr);
};

struct irqInfos {
	char used;
	struct irqfunctions *functions;
	unsigned char vector_number;
	int apic;
	unsigned char pinNr;
	unsigned char enabled;
};

extern struct irqfunctions legacyPIC;

extern struct irqInfos iInfos[NUM_IRQs];	// in zero.c

/*Prototypes */

int irq_enable();

/* loads the Interrupt Description Table */
/*   the first call of this function builds the table 
     following calls will only load the pointer in the IDT-Reg.*/
void irq_init_vectors();


/* sets the mask bit for irq in the corresponding (A)PIC */
void irq_disable();

#endif				/* no ASSEMBLER */

// APIC vector available to drivers: (vectors 0x31-0xfe)
#define CALL_FUNCTION_VECTOR 0xfa
#define CALL_FUNCTION_IRQNR  255

#define LAPIC_TIMER_VECTOR   0xe1
#define LAPIC_TIMER_IRQNR    200

#define SPURIOUS_APIC_VECTOR 0xfe
#define ERROR_APIC_VECTOR    0xfd

#endif
