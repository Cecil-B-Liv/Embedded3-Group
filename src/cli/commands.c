#include "commands.h"
#include "../ultil/stringUltil.h"
#include "../drivers/uart1.h"
#define MAX_COMMAND_NUMBER 4

const commandArr commands[MAX_COMMAND_NUMBER] = {
    { "help", "                  Show brief information of all commands", help},
    { "clear", "                 Clear screen", clear},
    { "braudRate", "             Allow the user to change the baudrate of current UART being used, include but not limited to: 9600, 19200, 38400, 57600, 115200 bits per second", braudRate},
    { "handShake", "             Allow the user to turn on/off CTS/RTS handsharking", handShake}
};


void cmdProcess(char* cmdBuff){
    // Split the original buffer too two, cmd and argument
    char* cmd = cmdBuff;
    char* arg;
    int spaceIndex = 0;
    for (spaceIndex; cmd[spaceIndex] != '\0'; spaceIndex++){
        if (cmd[spaceIndex] == ' '){
            cmd[spaceIndex] = '\0';
            arg = cmdBuff + spaceIndex + 1; // start of the argument
        }
    }

    // Command checking
    for (int i = 0; i < MAX_COMMAND_NUMBER; i++){
        if (strComp(cmdBuff, commands[i].name)){
            commands[i].cmdFunc(arg);
            return;
        }
    }

    uart_puts("\n[ERROR] ");
    uart_sendc('"');
    uart_puts(cmdBuff);
    uart_sendc('"');
    uart_puts(" is not recognized");
}

// Display description for each command avaiable on the board
void help(){
    uart_puts("\n\nFor more information on a specific command, type help <command-name>");
    for (int i = 0; i < MAX_COMMAND_NUMBER; i++){
        uart_puts("\n-");
        uart_puts(commands[i].name);
        uart_puts(commands[i].des);
    }
    uart_puts("\nFor more information on a specific command, type help <command-name>\n");
}
void helpC(){
    
}
void clear(){
    // Move the cursoer to somewhere user will not see
    for (int i = 0; i < 50; i++){
        uart_sendc('\n');
    }
}
void braudRate(){

}
void handShake(){

}

