#ifndef LED_DEBUG_H
#define LED_DEBUG_H

#ifndef QEMU
void led_signal(int count);
void led_on(void);
void led_off(void);
#endif

#endif
