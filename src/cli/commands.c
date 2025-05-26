#include "commands.h"

#include "../assets/testImg.h"
#include "../assets/background.h"
#include "../drivers/framebf.h"
#include "../drivers/mbox.h"
#include "../drivers/uart0.h"
#include "../util/stringUtil.h"
#include "../util/time.h"
#include "../util/utilsSap.h"
#include "../game/game.h"
#include "../assets/videoFrames/AK/akvideo.h"
#include "../assets/videoFrames/Cow/cowvideo.h"


#define myOs "FixingGoodOS>"
#define MAX_COMMAND_NUMBER 8

static int currentbaudrate = 115200;

const commandArr commands[MAX_COMMAND_NUMBER] = {
        {"help",         "                          Show brief information of all commands",                                                                                                       help},
        {"clear",        "                         Clear screen",                                                                                                                                  clear},
        {"showinfo",     "                      Show board revision and board MAC address",                                                                                                        showInfo},
        {"baudRate",     "                      Allow the user to change the baudRate of current UART being used, include but not limited to: 9600, 19200, 38400, 57600, 115200 bits per second", baudRate},
        {"handShake",    "                     Allow the user to turn on/off CTS/RTS handshaking",                                                                                                 handShake},
        {"teamDisplay",  "                   Display all team members name on the screen",                                                                                                         teamDisplay},
        {"videoDisplay", "                  Display the video",                                                                                                                                    videoDisplay},
        {"game",         "                          Enter the game menu",                                                                                                                         game} };

void cmdProcess(char* cmdBuff) {
    // Split the original buffer too two, cmd and argument
    char* cmd = cmdBuff;
    char* arg = 0; // NULL pointer
    for (int i = 0; cmd[i] != '\0'; i++) {
        if (cmd[i] == ' ') {
            cmd[i] = '\0';
            arg = cmdBuff + i + 1; // start of the argument
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
void help(char* arg) {
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

    uart_puts("\n");
}

void clear(char* arg) {
    if (arg != 0) {
        error(arg);
        return;
    }

    uart_puts("\033[2J"); // Clear screen
    uart_puts("\033[H");  // Move cursor to top

    // Backup: scroll buffer if ANSI fails
    for (int i = 0; i < 20; i++) {
        uart_sendc('\n');
    }
}

void showInfo(char* arg) {
    if (arg != 0) {
        error(arg);
        return;
    }

    // === Board Revision ===
    uart_puts("\n[INFO] Requesting BOARD REVISION...\n");

    mBuf[0] = 8 * 4;
    mBuf[1] = MBOX_REQUEST;

    mBuf[2] = 0x00010002; // Board Revision tag
    mBuf[3] = 4;
    mBuf[4] = 0;
    mBuf[5] = 0;

    mBuf[6] = MBOX_TAG_LAST;

    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        int rev = mBuf[5];
        uart_puts("\nBoard Revision: ");
        uart_hex(rev);
        uart_puts("\n");
        uart_puts(getBoardModel(rev));
    }
    else {
        uart_puts("\n[ERROR] Failed to get board revision.\n");
    }

    // === MAC Address ===
    uart_puts("\n[INFO] Requesting MAC ADDRESS...\n");

    mBuf[0] = 9 * 4;
    mBuf[1] = MBOX_REQUEST;

    mBuf[2] = 0x00010003; // MAC Address tag
    mBuf[3] = 6;
    mBuf[4] = 0;
    mBuf[5] = 0; // MAC part 1
    mBuf[6] = 0; // MAC part 2

    mBuf[7] = MBOX_TAG_LAST;

    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        unsigned char* mac = (unsigned char*)&mBuf[5];
        uart_puts("\nMAC Address: ");
        for (int i = 0; i < 6; i++) {
            uart_mac_formatter(mac[i]);
            if (i < 5)
                uart_puts(":");
        }
        uart_puts("\n");
    }
    else {
        uart_puts("\n[ERROR] Failed to get MAC address.\n");
    }
}

void baudRate(char* arg) {
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
    case 115200:uart_puts("\nBaud rate set successfully.\n");
        currentbaudrate = baud;
        uart_puts("Now using ");
        uart_dec(baud);
        uart_puts(" baudrate\n");
        wait_msec(100); // ensure no fuckery
        uart_setBaudrate(baud);
        break;
    default:uart_puts("\n[ERROR] Unsupported baud rate.\n");
        break;
    }
}

void handShake(char* arg) {
    unsigned int handshakemode = 0;
    if (!arg || arg[0] == '\0') {
        error(arg);
        uart_puts("\n");
        return;
    }

    if (strComp(arg, "On") || strComp(arg, "on")) {
        handshakemode = 1;
    }
    else if (strComp(arg, "Off") || strComp(arg, "off")) {
        handshakemode = 0;
    }

    if (handshakemode) {
        wait_msec(100);
        uart_init(UART0_MODE_HANDSHAKE, currentbaudrate);
        uart_puts("\nSet Handshake On Succesfully.\n");
        return;
    }
    wait_msec(100);
    uart_init(UART0_MODE_NORMAL, currentbaudrate);
    uart_puts("\nSet Handshake Off Succesfully.\n");
    return;
}

void error(char* error) {
    uart_puts("\n[ERROR] ");
    uart_sendc('"');
    uart_puts(error);
    uart_sendc('"');
    uart_puts(" is not recognized");
}

char* getBoardModel(int rev) {
    // New-style revision format (bit 23 set)
    if ((rev & (1 << 23)) != 0) {
        int model = (rev >> 4) & 0xFF;

        switch (model) {
        case 0x00:return "Model: A\nRAM: 256MB\nRevision: 1.0";
        case 0x01:return "Model: B\nRAM: 256MB\nRevision: 1.0/1.1";
        case 0x02:return "Model: A+\nRAM: 256MB or 512MB\nRevision: 1.1";
        case 0x03:return "Model: B+\nRAM: 512MB\nRevision: 1.2";
        case 0x04:return "Model: 2B\nRAM: 1GB\nRevision: 1.0";
        case 0x06:return "Model: CM1\nRAM: 512MB\nRevision: 1.0";
        case 0x08:return "Model: 3B\nRAM: 1GB\nRevision: 1.2";
        case 0x09:return "Model: Zero\nRAM: 512MB\nRevision: 1.2";
        case 0x0C:return "Model: Zero W\nRAM: 512MB\nRevision: 1.1";
        case 0x0D:return "Model: 3B+\nRAM: 1GB\nRevision: 1.3";
        case 0x0E:return "Model: 3A+\nRAM: 512MB\nRevision: 1.0";
        case 0x10:return "Model: CM3+\nRAM: 1GB\nRevision: 1.0";
        case 0x11:return "Model: 4B\nRAM: 1GB\nRevision: 1.1";
        case 0x12:return "Model: 4B\nRAM: 2GB\nRevision: 1.2";
        case 0x13:return "Model: 4B\nRAM: 4GB\nRevision: 1.2";
        case 0x14:return "Model: 4B\nRAM: 8GB\nRevision: 1.4";
        case 0x15:return "Model: 400\nRAM: 4GB\nRevision: 1.0";
        case 0x19:return "Model: Zero 2 W\nRAM: 512MB or 1GB\nRevision: 1.0";
        case 0x1A:return "Model: 3A+ (alternate)\nRAM: 512MB\nRevision: 1.1";
        default:return "Model: Unknown (new-style revision)";
        }
    }

    // Old-style revision format
    switch (rev) {
    case 0x0002:return "Model: Model B Rev 1\nRAM: 256MB\nRevision: none";
    case 0x0003:return "Model: Model B Rev 1 (ECN0001)\nRAM: 256MB\nRevision: none";
    case 0x0004:
    case 0x0005:
    case 0x0006:return "Model: Model B Rev 2\nRAM: 256MB\nRevision: none";
    case 0x0007:
    case 0x0008:
    case 0x0009:return "Model: Model A\nRAM: 256MB\nRevision: none";
    case 0x000d:
    case 0x000e:
    case 0x000f:return "Model: Model B Rev 2\nRAM: 512MB\nRevision: none";
    case 0x0010:
    case 0x0013:
    case 0x900032:return "Model: Model B+\nRAM: 512MB\nRevision: none";
    case 0x0011:return "Model: Compute Module\nRAM: 512MB\nRevision: none";
    case 0x0014:return "Model: Compute Module (Embest)\nRAM: 512MB\nRevision: none";
    case 0x0012:return "Model: Model A+\nRAM: 256MB\nRevision: none";
    case 0x0015:return "Model: Model A+ (Embest)\nRAM: 256MB or 512MB\nRevision: none";
    case 0xa01041:return "Model: Pi 2 Model B v1.1 (Sony UK)\nRAM: 1GB\nRevision: 1.1";
    case 0xa21041:return "Model: Pi 2 Model B v1.1 (Embest)\nRAM: 1GB\nRevision: 1.1";
    case 0xa22042:return "Model: Pi 2 Model B v1.2\nRAM: 1GB\nRevision: 1.2";
    case 0x900092:return "Model: Pi Zero v1.2\nRAM: 512MB\nRevision: 1.2";
    case 0x900093:return "Model: Pi Zero v1.3\nRAM: 512MB\nRevision: 1.3";
    case 0x9000C1:return "Model: Pi Zero W\nRAM: 512MB\nRevision: 1.1";
    case 0xa02082:return "Model: Pi 3 Model B (Sony UK)\nRAM: 1GB\nRevision: 1.2";
    case 0xa22082:return "Model: Pi 3 Model B (Embest)\nRAM: 1GB\nRevision: 1.2";
    case 0xa020d3:return "Model: Pi 3 Model B+ (Sony UK)\nRAM: 1GB\nRevision: 1.3";
    case 0xa03111:return "Model: Pi 4 1GB v1.1 (Sony UK)\nRAM: 1GB\nRevision: 1.1";
    case 0xb03111:return "Model: Pi 4 2GB v1.1 (Sony UK)\nRAM: 2GB\nRevision: 1.1";
    case 0xb03112:return "Model: Pi 4 2GB v1.2 (Sony UK)\nRAM: 2GB\nRevision: 1.2";
    case 0xb03114:return "Model: Pi 4 2GB v1.4 (Sony UK)\nRAM: 2GB\nRevision: 1.4";
    case 0xc03111:return "Model: Pi 4 4GB v1.1 (Sony UK)\nRAM: 4GB\nRevision: 1.1";
    case 0xc03112:return "Model: Pi 4 4GB v1.2 (Sony UK)\nRAM: 4GB\nRevision: 1.2";
    case 0xc03114:return "Model: Pi 4 4GB v1.4 (Sony UK)\nRAM: 4GB\nRevision: 1.4";
    case 0xd03114:return "Model: Pi 4 8GB v1.4 (Sony UK)\nRAM: 8GB\nRevision: 1.4";
    case 0xc03130:return "Model: Pi 400 4GB v1.0\nRAM: 4GB\nRevision: 1.0";
    case 0x902120:return "Model: Pi Zero 2 W 1GB v1.0\nRAM: 1GB\nRevision: 1.0";
    default:return "Model: Unknown\nRAM: Unknown\nRevision: Unknown";
    }
}

void teamDisplay() {
    drawImg2(background, 0, 0, 1024, 768);

    drawString(30, 240, "\nKIM NHAT ANH         s3978831", 0x00FF0000, 3); // red
    drawString(30, 290, "\nTRAN QUANG MINH      s3988876", 0x0000FF00, 3); // green
    drawString(30, 340, "\nHUYNH NGOC TAI       s3978680", 0x000000FF, 3); // blue
    drawString(30, 390, "\nVU THIEN MINH HAO    s3938011", 0x00FFFF00, 3); // yellow
    uart_puts("\n");
}

void videoDisplay(char* arg) {
    if (!arg || arg[0] == '\0') {
        error(arg);
        uart_puts("\n");
        return;
    }

    if (strComp(arg, "ak")) {
        int size = sizeof(akvideo_allArray) / sizeof(akvideo_allArray[0]);

        for (int i = 0; i < size; i++) {
            drawImg2(akvideo_allArray[i], 200, 215, 600, 338);
            wait_msec(100); // 1000 ms / 31 frames ≈ 32 ms per frame
        }
        return;
    }
    else {
        int size = sizeof(cowvideo_allArray) / sizeof(cowvideo_allArray[0]);

        for (int i = 0; i < size; i++) {
            drawImg2(cowvideo_allArray[i], 200, 215, 500, 300);
            wait_msec(100); // 1000 ms / 31 frames ≈ 32 ms per frame
        }
    }
    return;

}

void game() {
    gameMenu();
}