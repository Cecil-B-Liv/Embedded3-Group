#include "commands.h"
#include "../ultil/stringUltil.h"
#include "../drivers/uart1.h"
#define COMMAND_NUMBER 4


const commandArr commands[COMMAND_NUMBER] = {
    { "help", "- Show brief information of all commands\n- Example: FixingGood> help\n", help},
    { "clear", "- Clear screen\n- Example: MyOS> clear\n", clear},
    { "braudRate", "- Allow the user to change the baudrate of current UART being used.\nThe command support various baud rates include but not limited to: 9600, 19200, 38400, 57600, 115200 bits per second.\n", braudRate},
    { "handShake", "- Allow the user to turn on/off CTS/RTS handsharking\n", handShake}
};


void cmdProcess(char* cmdBuff){
    for (int i = 0; i < COMMAND_NUMBER; i++){
        if (strComp(cmdBuff, commands[i].name)){
            commands[i].cmdFunc();
            return;
        }
    }

    // If the command is not recognized
    uart_puts("\n[ERROR] ");
    uart_sendc('"');
    uart_puts(cmdBuff);
    uart_sendc('"');
    uart_puts(" is not recognized\n");
}

void help(){

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