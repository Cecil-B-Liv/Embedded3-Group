#include "../drivers/uart1.h"
#include "cli.h"
#include "commands.h"

#define MAX_COMMAND_SIZE 100
#define myOs "FixingGoodOS>"

// Static varriable to keep track of the command buffer
static char commandBuffer[MAX_COMMAND_SIZE];
static int cbIndex = 0; // pointer of command buffer

void cli_welcome(){
    uart_puts("==============================================================================================================\n\n");
    uart_puts("8888888888 8888888 Y88b   d88P 8888888 888b    888  .d8888b.      .d8888b.   .d88888b.   .d88888b.  8888888b.  \n");
    uart_puts("888          888    Y88b d88P    888   8888b   888 d88P  Y88b    d88P  Y88b d88P   Y88b d88P   Y88b 888   Y88b \n");
    uart_puts("888          888     Y88o88P     888   88888b  888 888    888    888    888 888     888 888     888 888    888 \n");
    uart_puts("8888888      888      Y888P      888   888Y88b 888 888           888        888     888 888     888 888    888 \n");
    uart_puts("888          888      d888b      888   888 Y88b888 888  88888    888  88888 888     888 888     888 888    888 \n");
    uart_puts("888          888     d88888b     888   888  Y88888 888    888    888    888 888     888 888     888 888    888 \n");
    uart_puts("888          888    d88P Y88b    888   888   Y8888 Y88b  d88P    Y88b  d88P Y88b. .d88P Y88b. .d88P 888  .d88P \n");
    uart_puts("888        8888888 d88P   Y88b 8888888 888    Y888  Y8888P88      Y8888P88   Y88888P      Y88888P   8888888P   \n");
    uart_puts("\r\n");

    uart_puts("==============================================================================================================\n");
    uart_puts("                                          Developer Team\r\n");
    uart_puts("--------------------------------------------------------------------------------------------------------------\n");
    uart_puts("  - Kim Nhat Anh       | ID: s3978831\r\n");
    uart_puts("  - Huynh Ngoc Tai     | ID: s3978680\r\n");
    uart_puts("  - Tran Quang Minh    | ID: s3988776\r\n");
    uart_puts("  - asdfadfasdf        | ID: 111111111\r\n");
    uart_puts("==============================================================================================================\n");
    uart_puts("\n");
    uart_puts(myOs);
}

void cli_process(){
    // Get the user input
    char c = uart_getc();

    //Process the user input base on different case
    switch (c){

        // User press ENTER
        case '\n':
            // Call the commands processer function
            cmdProcess(commandBuffer);

            // Clear buffer after process the command 
            clearBuff();
            break;
        
        // User delete character
        case '\b':
            // Nothing to delete
            if (cbIndex == 0){ 
                break;
            }

            // Visual display delete for user
            uart_sendc('\b');
            uart_sendc(' ');
            uart_sendc('\b');

            // Modify the buffer
            cbIndex--;
            commandBuffer[cbIndex] = '\0';
            break;
        // Normal input
        default:
            // Maximum command size reached
            if (cbIndex > MAX_COMMAND_SIZE - 1){
                break; // do nothing
            }
            // Add the new character to the buffer
            commandBuffer[cbIndex] = c;
            cbIndex++;
            // Print the user input to the terminal
            uart_sendc(c);
    }
}

// Clear buffer and create now input line
void clearBuff(){
  // Clear the command buffer and reset the index
    for (int i = 0; i < cbIndex; i++) {  
        commandBuffer[i] = '\0';
    }
    cbIndex = 0;
    uart_sendc('\n');
    uart_puts(myOs);
}