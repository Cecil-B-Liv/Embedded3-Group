#include "cli.h"
#include "../drivers/uart0.h"
#include "../util/stringUtil.h"
#include "commands.h"

#define MAX_COMMAND_SIZE 100
#define myOs "FixingGoodOS>"
#define MAX_HISTORY 10

// Static varriable to keep track of the command buffer
static char commandBuffer[MAX_COMMAND_SIZE];
static int cbIndex = 0;  // pointer of command buffer

// Tab function varriable
static char *commands[] = {
        "help", "clear", "showinfo", "baudRate", "handShake", "teamDisplay", "videoDisplay"};
static int NUM_COMMANDS = (sizeof(commands) / sizeof(commands[0]));
static char *matched = 0;     // place holder for the matched checking
static int matchedFound = 0;  // Flag to check if matched found

static uint8_t escape_seq = 0;

// History varriable
static char history[MAX_HISTORY][MAX_COMMAND_SIZE];
static int historyCount = 0;
static int historyIndex = -1;

void cli_welcome() {
    uart_puts(
            "=========================================================================================="
            "====================\n\n");
    uart_puts(
            "8888888888 8888888 Y88b   d88P 8888888 888b    888  .d8888b.      .d8888b.   .d88888b.   "
            ".d88888b.  8888888b.  \n");
    uart_puts(
            "888          888    Y88b d88P    888   8888b   888 d88P  Y88b    d88P  Y88b d88P   Y88b "
            "d88P   Y88b 888   Y88b \n");
    uart_puts(
            "888          888     Y88o88P     888   88888b  888 888    888    888    888 888     888 "
            "888     888 888    888 \n");
    uart_puts(
            "8888888      888      Y888P      888   888Y88b 888 888           888        888     888 "
            "888     888 888    888 \n");
    uart_puts(
            "888          888      d888b      888   888 Y88b888 888  88888    888  88888 888     888 "
            "888     888 888    888 \n");
    uart_puts(
            "888          888     d88888b     888   888  Y88888 888    888    888    888 888     888 "
            "888     888 888    888 \n");
    uart_puts(
            "888          888    d88P Y88b    888   888   Y8888 Y88b  d88P    Y88b  d88P Y88b. .d88P "
            "Y88b. .d88P 888  .d88P \n");
    uart_puts(
            "888        8888888 d88P   Y88b 8888888 888    Y888  Y8888P88      Y8888P88   Y88888P      "
            "Y88888P   8888888P   \n");
    uart_puts("\r\n");

    uart_puts(
            "=========================================================================================="
            "====================\n");
    uart_puts("                                          Developer Team\r\n");
    uart_puts(
            "------------------------------------------------------------------------------------------"
            "--------------------\n");
    uart_puts("  - Kim Nhat Anh       | ID: s3978831\r\n");
    uart_puts("  - Huynh Ngoc Tai     | ID: s3978680\r\n");
    uart_puts("  - Tran Quang Minh    | ID: s3988776\r\n");
    uart_puts("  - Vu Thien Minh Hao  | ID: s3938011\r\n");
    uart_puts(
            "=========================================================================================="
            "====================\n");
    uart_puts("\n");
    uart_puts(myOs);
}

void cli_process() {
    // Get the user input
    int c = uart_get_escape_sequence();

    // Process the user input base on different case
    switch (c) {
        case '-':  // Go backward in history
            if (historyIndex > 0) {
                historyIndex--;
                clearDisplay();
                clearBuff(1);
                strCopy(commandBuffer, history[historyIndex]);
                cbIndex = strLen(commandBuffer);
                uart_puts(commandBuffer);
            }
            break;

        case '=':  // Go forward in history
            if (historyIndex < historyCount - 1) {
                historyIndex++;
                clearDisplay();
                clearBuff(1);
                strCopy(commandBuffer, history[historyIndex]);
                cbIndex = strLen(commandBuffer);
                uart_puts(commandBuffer);
            } else {
                // Clear input if at the end
                clearDisplay();
                clearBuff(1);
            }
            break;

        case KEY_ARROW_UP:  // Go back in history
            if (historyIndex > 0) {
                historyIndex--;
                clearDisplay();
                clearBuff(1);
                strCopy(commandBuffer, history[historyIndex]);
                cbIndex = strLen(commandBuffer);
                uart_puts(commandBuffer);
            }
            break;

        case KEY_ARROW_DOWN:  // Go forward in history
            if (historyIndex < historyCount - 1) {
                historyIndex++;
                clearDisplay();
                clearBuff(1);
                strCopy(commandBuffer, history[historyIndex]);
                cbIndex = strLen(commandBuffer);
                uart_puts(commandBuffer);
            } else {
                // Clear input if at the end
                clearDisplay();
                clearBuff(1);
            }
            break;
        case KEY_ARROW_LEFT:
        case KEY_ARROW_RIGHT:
            // Reset cursor & redraw prompt + current input
            uart_puts("\r\033[K");
            uart_puts(myOs);
            uart_puts(commandBuffer);
            break;

            // User using autofill by pressing tab
        case '\t':
            // Loop to check the matched command
            for (int i = 0; i < NUM_COMMANDS; i++) {
                if (startsWith(commands[i], commandBuffer)) {
                    matched = commands[i];
                    matchedFound = 1;
                    break;
                }
            }

            // Clear the display
            clearDisplay();

            // clear the buffer
            clearBuff(1);

            // Promt the matched command;
            for (int i = 0; matched[i] != '\0'; i++) {
                commandBuffer[cbIndex++] = matched[i];
                uart_sendc(matched[i]);
            }

            matchedFound = 0;  // reset the flag
            break;
            // User press ENTER
        case '\n':
            // Call the commands processer function
            cmdProcess(commandBuffer);

            if (cbIndex > 0) {
                if (historyCount < MAX_HISTORY) {
                    strCopy(history[historyCount++], commandBuffer);
                } else {
                    for (int i = 1; i < MAX_HISTORY; i++) {
                        strCopy(history[i - 1], history[i]);
                    }
                    strCopy(history[MAX_HISTORY - 1], commandBuffer);
                }
            }
            historyIndex = historyCount;
            clearBuff(0);

            break;

            // User delete character
        case 127:
        case '\b':
            // Nothing to delete
            if (cbIndex == 0) {
                break;
            }

            cbIndex--;
            commandBuffer[cbIndex] = '\0';

            // Redraw everything after deletion
            uart_puts("\r\033[K");       // Clear line
            uart_puts(myOs);             // Print prompt
            uart_puts(commandBuffer);    // Print updated buffer
            break;

            // Prevent cursor from moving
        case 0x1B:  // Escape character
            escape_seq = 1;
            break;
            // Normal input
        default:
            // Maximum command size reached
            if (cbIndex > MAX_COMMAND_SIZE - 1) {
                break;  // do nothing
            }
            // Add the new character to the buffer
            commandBuffer[cbIndex] = c;
            cbIndex++;
            // Print the user input to the terminal
            uart_sendc(c);
    }
}

// Clear buffer and create now input line
void clearBuff(int isTab) {
    // Clear the command buffer and reset the index
    for (int i = 0; i < cbIndex; i++) {
        commandBuffer[i] = '\0';
    }
    cbIndex = 0;

    // if the function called in the tab then dont print the next os
    if (isTab) return;
    uart_sendc('\n');
    uart_puts(myOs);
}

// clear all the visual inputed on the screen
void clearDisplay() {
//    for (int i = 0; i < cbIndex; i++) {
//        uart_sendc('\b');  // move back
//        uart_sendc(' ');   // overwrite with space
//        uart_sendc('\b');  // move back again
//    }
//    uart_puts("\r");         // Go to beginning of the line
    uart_puts("\r\033[K");
    uart_puts(myOs);
}