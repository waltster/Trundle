#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
 
#include <kernel/tty.h>
 
#include "vga.h"
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;
 
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;
 
void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}
 
void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_scroll(int line) {
    memmove(&terminal_buffer[0], &terminal_buffer[VGA_WIDTH], 
            sizeof(uint16_t) * VGA_WIDTH * (VGA_HEIGHT - 1));
    memset(&terminal_buffer[VGA_WIDTH * (VGA_HEIGHT - 1)], 0, 
            sizeof(uint16_t) * VGA_WIDTH);
}
 
void terminal_delete_last_line() {
	size_t x;
    int *ptr;
 
	for(x = 0; x < VGA_WIDTH * 2; x++) {
		ptr = (int*) (0xB8000 + (VGA_WIDTH * 2) * (VGA_HEIGHT - 1) + x);
		*ptr = 0;
	}
}
 
void terminal_putchar(char c) {
	size_t line;
	unsigned char uc = c;

    if ((c == 0x08 && terminal_column) || (c == '\b' && terminal_column)) {
        terminal_column--;
    } else if (c == 0x09 || c == '\t') {
        terminal_column = (terminal_column + 8) & ~(7);
    } else if (c == '\r') {
        terminal_column = 0;
    } else if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else if (c >= ' ') {
        terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
        terminal_column++;
    }

	
    if (terminal_column >= VGA_WIDTH) {
		terminal_column = 0;
        terminal_row++;
    }

	if (terminal_row >= VGA_HEIGHT) {
        int i = 0;

        memcpy(terminal_buffer[0], terminal_buffer[80], VGA_WIDTH * 24);

        for (i = 0; i < 24 * 80; i++) {
            terminal_buffer[i] = terminal_buffer[i + 80];
        }

        for (i = 0; i < 80; i++) {
            terminal_putentryat(' ', terminal_color, i, VGA_HEIGHT - 1);
        }

        terminal_row = VGA_HEIGHT - 1;
	}
}
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) {
		terminal_putchar(data[i]);
    }
}
 
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
