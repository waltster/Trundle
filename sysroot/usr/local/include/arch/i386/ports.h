#ifndef _PORTS_H
#define _PORTS_H 1

extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

#endif
