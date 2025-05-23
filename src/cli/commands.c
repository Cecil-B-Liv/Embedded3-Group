#include "commands.h"

#include "../assets/testImg.h"
#include "../assets/video.h"
#include "../drivers/framebf.h"
#include "../drivers/mbox.h"
#include "../drivers/uart0.h"
#include "../util/stringUtil.h"
#include "../util/time.h"
#include "../util/utilsSap.h"

#define MAX_COMMAND_NUMBER 7

static int currentbaudrate = 115200;

const commandArr commands[MAX_COMMAND_NUMBER] = {
    {"help", "                          Show brief information of all commands", help},
    {"clear", "                         Clear screen", clear},
    {"showinfo", "                      Show board revision and board MAC address", showInfo},
    {"baudRate","                      Allow the user to change the baudRate of current UART being used, include but not limited to: 9600, 19200, 38400, 57600, 115200 bits per second", baudRate},
    {"handShake", "                     Allow the user to turn on/off CTS/RTS handshaking", handShake},
    {"teamDisplay", "                   Display all team members name on the screen", teamDisplay},
    {"videoDisplay", "                  Display the video", videoDisplay}};

void cmdProcess(char *cmdBuff) {
    // Split the original buffer too two, cmd and argument
    char *cmd = cmdBuff;
    char *arg = 0;  // NULL pointer
    for (int i = 0; cmd[i] != '\0'; i++) {
        if (cmd[i] == ' ') {
            cmd[i] = '\0';
            arg = cmdBuff + i + 1;  // start of the argument
            break;
        }
    }

    // Command checking
    for (int i = 0; i < MAX_COMMAND_NUMBER; i++) {
        if (strComp(cmdBuff, commands[i].name)) {
            commands[i].cmdFunc(arg);
            return;
        }
    }

    error(cmd);
}

// Display description for each command avaiable on the board
void help(char *arg) {
    // If help <command_name> is used
    if (arg != 0) {
        for (int i = 0; i < MAX_COMMAND_NUMBER; i++) {
            if (strComp(arg, commands[i].name)) {
                uart_puts("\n");
                uart_puts(commands[i].name);
                uart_puts(commands[i].des);
                return;
            }
        }

        // if the argument is not reconigse
        error(arg);
        return;
    }
    uart_puts("\n\nFor more information on a specific command, type help <command-name>");

    uart_puts("\n-help <command_name>           Show full information of a specific command");
    for (int i = 0; i < MAX_COMMAND_NUMBER; i++) {
        uart_puts("\n-");
        uart_puts(commands[i].name);
        uart_puts(commands[i].des);
    }
}

void clear(char *arg) {
    // This command accept no argument
    if (arg != 0) {
        error(arg);
        return;
    }

    // move the cursor to the location where user cannot see
    for (int i = 0; i < 50; i++) {
        uart_sendc('\n');
    }
}

void showInfo(char *arg) {
    // This command accept no argument
    if (arg != 0) {
        error(arg);
        return;
    }

    // Mail Box Request
    // mailbox data buffer
    mBuf[0] = 11 * 4;        // Message Buffer Size in bytes
    mBuf[1] = MBOX_REQUEST;  // Message Request Code (this is a request message)

    mBuf[2] = 0x00010002;  // Board Revision
    mBuf[3] = 4;           // Value buffer size in bytes (max of request and response lengths)
    mBuf[4] = 0;           // REQUEST CODE = 0
    mBuf[5] = 0;           // Board Revision Value

    mBuf[6] = 0x00010003;  // Board MAC Adress
    mBuf[7] = 6;           // Value buffer size in bytes (max of request and response lengths)
    mBuf[8] = 0;           // REQUEST CODE = 0
    mBuf[9] = 0;           // Board MAC address Value

    mBuf[10] = MBOX_TAG_LAST;

    // Call the Mail Box and get responses
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        // Varriable to hold the value of board revision and MAC Adress
        int boardRev = mBuf[5];
        char *mac = (char *)&mBuf[9];

        // Board Revision Information
        uart_puts("\nBoard Revision: ");
        uart_hex(boardRev);
        uart_puts("\n");
        uart_puts(getBoardModel(boardRev));

        // MAC Adress (6 values format)
        uart_puts("\nMAC Address: ");
        for (int i = 0; i < 6; i++) {
            uart_mac_formater((unsigned char)mac[i]);
            if (i < 5) uart_puts(":");
        }
    }
}

void baudRate(char *arg) {
    if (!arg || arg[0] == '\0') {
        uart_puts("\n[ERROR] No baud rate specified. Usage: baudrate <baud>\n");
        return;
    }

    if (!is_all_digits(arg)) {
        uart_puts("\n[ERROR] Input must be numeric only.\n");
        return;
    }

    int baud = my_atoi(arg);

    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
            uart_puts("\nBaud rate set successfully.\n");
            currentbaudrate = baud;
            uart_puts("Now using ");
            uart_dec(baud);
            uart_puts(" baudrate\n");
            wait_msec(100);  // ensure no fuckery
            uart_setBaudrate(baud);
            break;
        default:
            uart_puts("\n[ERROR] Unsupported baud rate.\n");
            break;
    }
}

void handShake(char *arg) {
    unsigned int handshakemode = 0;
    if (!arg || arg[0] == '\0' ) {
        error(arg);
        return;
    }

    if (strComp(arg, "On") || strComp(arg, "on")) {
        handshakemode = 1;
    }

    if (strComp(arg, "Off") || strComp(arg, "off")) {
        handshakemode = 0;
    }

    if (handshakemode) {
        uart_init(UART0_MODE_HANDSHAKE, currentbaudrate);
        uart_puts("\nSet Handshake On Succesfully.\n");
        return;
    }
    uart_init(UART0_MODE_NORMAL, currentbaudrate);
    uart_puts("\nSet Handshake Off Succesfully.\n");
    return;
}

void error(char *error) {
    uart_puts("\n[ERROR] ");
    uart_sendc('"');
    uart_puts(error);
    uart_sendc('"');
    uart_puts(" is not recognized");
}

// use to store the information of the board revision
char *getBoardModel(int rev) {
    switch (rev) {
        case 0x0002:
            return "Model: Model B Rev 1\nRAM: 256MB\nRevision: none";
        case 0x0003:
            return "Model: Model B Rev 1 (ECN0001, no fuses, D14 removed)\nRAM: 256MB\nRevision: "
                   "none";
        case 0x0004:
        case 0x0005:
        case 0x0006:
            return "Model: Model B Rev 2\nRAM: 256MB\nRevision: none";
        case 0x0007:
        case 0x0008:
        case 0x0009:
            return "Model: Model A\nRAM: 256MB\nRevision: none";
        case 0x000d:
        case 0x000e:
        case 0x000f:
            return "Model: Model B Rev 2\nRAM: 512MB\nRevision: none";
        case 0x0010:
        case 0x0013:
        case 0x900032:
            return "Model: Model B+\nRAM: 512MB\nRevision: none";
        case 0x0011:
            return "Model: Compute Module\nRAM: 512MB\nRevision: none";
        case 0x0014:
            return "Model: Compute Module (Embest, China)\nRAM: 512MB\nRevision: none";
        case 0x0012:
            return "Model: Model A+\nRAM: 256MB\nRevision: none";
        case 0x0015:
            return "Model: Model A+ (Embest)\nRAM: 256MB or 512MB\nRevision: none";
        case 0xa01041:
            return "Model: Pi 2 Model B v1.1 (Sony, UK)\nRAM: 1GB\nRevision: 1.1";
        case 0xa21041:
            return "Model: Pi 2 Model B v1.1 (Embest, China)\nRAM: 1GB\nRevision: 1.1";
        case 0xa22042:
            return "Model: Pi 2 Model B v1.2\nRAM: 1GB\nRevision: 1.2";
        case 0x900092:
            return "Model: Pi Zero v1.2\nRAM: 512MB\nRevision: 1.2";
        case 0x900093:
            return "Model: Pi Zero v1.3\nRAM: 512MB\nRevision: 1.3";
        case 0x9000C1:
            return "Model: Pi Zero W\nRAM: 512MB\nRevision: 1.1";
        case 0xa02082:
            return "Model: Pi 3 Model B (Sony, UK)\nRAM: 1GB\nRevision: 1.2";
        case 0xa22082:
            return "Model: Pi 3 Model B (Embest, China)\nRAM: 1GB\nRevision: 1.2";
        case 0xa020d3:
            return "Model: Pi 3 Model B+ (Sony, UK)\nRAM: 1GB\nRevision: 1.3";
        case 0xa03111:
            return "Model: Pi 4 1GB v1.1 (Sony, UK)\nRAM: 1GB\nRevision: 1.1";
        case 0xb03111:
            return "Model: Pi 4 2GB v1.1 (Sony, UK)\nRAM: 2GB\nRevision: 1.1";
        case 0xb03112:
            return "Model: Pi 4 2GB v1.2 (Sony, UK)\nRAM: 2GB\nRevision: 1.2";
        case 0xb03114:
            return "Model: Pi 4 2GB v1.4 (Sony, UK)\nRAM: 2GB\nRevision: 1.4";
        case 0xc03111:
            return "Model: Pi 4 4GB v1.1 (Sony, UK)\nRAM: 4GB\nRevision: 1.1";
        case 0xc03112:
            return "Model: Pi 4 4GB v1.2 (Sony, UK)\nRAM: 4GB\nRevision: 1.2";
        case 0xc03114:
            return "Model: Pi 4 4GB v1.4 (Sony, UK)\nRAM: 4GB\nRevision: 1.4";
        case 0xd03114:
            return "Model: Pi 4 8GB v1.4 (Sony, UK)\nRAM: 8GB\nRevision: 1.4";
        case 0xc03130:
            return "Model: Pi 400 4GB v1.0\nRAM: 4GB\nRevision: 1.0";
        case 0x902120:
            return "Model: Pi Zero 2 W 1GB v1.0\nRAM: 1GB\nRevision: 1.0";
        default:
            return "Model: Unknown\nRAM: Unknown\nRevision: Unknown";
    }
}

void teamDisplay() {
    drawImg(background, 0, 0, 1024, 768);

    drawString(30, 240, "\nKIM NHAT ANH         s3978831", 0x00FF0000, 3);   // red
    drawString(30, 290, "\nTRAN QUANG MINH      s3988876", 0x0000FF00, 3);   // green
    drawString(30, 340, "\nHUYNH NGOC TAI       s3978680", 0x000000FF, 3);  // blue
    drawString(30, 390, "\nVU THIEN MINH HAO    s3938011", 0x00FFFF00, 3);  // yellow
    uart_puts("\n");
}

void videoDisplay(char *arg) {
    if (arg != 0) {
        error(arg);
        return;
    }

    drawImg(background, 0, 0, 1024, 768);

    for (int i = 0; i <= 30; i++) {
        drawImg(VIDEO_ARRAY[i], 200, 215, 600, 338);
        delay_ms(50);  // 1000 ms / 31 frames ≈ 32 ms per frame
    }
    uart_puts("\n");
}