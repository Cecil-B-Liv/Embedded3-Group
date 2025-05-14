#include "../drivers/uart1.h"
#include "../drivers/mbox.h"
#include "../drivers/framebf.h"
#include "../cli/cli.h"

void main()
{
    // set up serial console
	uart_init();

	// cli print message
	cli_welcome();

	// echo everything back
	// while(1) {
	// 	//read each char
	// 	char c = uart_getc();

	// 	//send back 
	// 	uart_sendc(c);
	// }
}