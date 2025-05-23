#include "../drivers/uart0.h"
#include "../drivers/mbox.h"
#include "../drivers/framebf.h"
#include "../cli/cli.h"

void main()
{
    // set up serial console
    uart_init(UART0_MODE_NORMAL, 115200);

    // setup the framebuffer
    framebf_init();

    // cli print message
    cli_welcome();

    // CLI processing
    while (1)
    {
        cli_process();
    }
}