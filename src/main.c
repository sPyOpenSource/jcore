#define GPIO4_BASE         0x4805D000
#define GPIO_OE            0x134
#define GPIO_SETDATAOUT    0x194
#define GPIO_CLEARDATAOUT  0x190
#define LED_PIN            (1 << 22) // Voorbeeld: USR3 LED op GPIO4_22

void main(void) {
    // 1. Configureer de pin als OUTPUT (schrijf een 0 naar de bit in het OE-register)
    volatile unsigned int *gpio_oe = (unsigned int *)(GPIO4_BASE + GPIO_OE);
    *gpio_oe &= ~LED_PIN;

    volatile unsigned int *set_reg = (unsigned int *)(GPIO4_BASE + GPIO_SETDATAOUT);
    volatile unsigned int *clear_reg = (unsigned int *)(GPIO4_BASE + GPIO_CLEARDATAOUT);

    while(1) {
        *set_reg = LED_PIN;   // LED aan
        for(volatile int i = 0; i < 500000; i++); // Bare-metal delay loop
        *clear_reg = LED_PIN; // LED uit
        for(volatile int i = 0; i < 500000; i++);
    }
}
