#include "../drivers/uart1.h"
#include "../drivers/mbox.h"
#include "../drivers/framebf.h"

void main()
{
    // set up serial console
	uart_init();

	// say hello
	uart_puts("\n\nHello World!\n");

	// Initialize frame buffer
	framebf_init();

	// Draw the screen background
	// Note: you can use clock picker tools to get the color value
	drawRectARGB32(0, 0, 1023, 767, 0x00CAEEFB, 1);

	// Draw the lines (black)
	drawLine(0, 0,  1023, 767, 0x00000000);
	drawLine(512, 384, 1023, 0, 0x00000000);

	// Draw the circle (black)
	drawLCircle(512, 384, 200, 0x00000000, 0);


	// echo everything back
	while(1) {
		//read each char
		char c = uart_getc();

		//send back 
		uart_sendc(c);
	}
}