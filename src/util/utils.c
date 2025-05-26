#include "font.h"
#include "../drivers/framebf.h"
#include "../drivers/uart0.h"
#include "utils.h"

/* Functions to display text on the screen */
// NOTE: zoom = 0 will not display the character
void drawChar(unsigned char ch, int x, int y, unsigned int attr, int zoom) {
    unsigned char *glyph = (unsigned char *) &font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i = 1; i <= (FONT_HEIGHT * zoom); i++) {
        for (int j = 0; j < (FONT_WIDTH * zoom); j++) {
            unsigned char mask = 1 << (j / zoom);
            if (*glyph & mask) { //only draw pixels belong to the character glyph
                drawPixelARGB32(x + j, y + i, attr);
            }
        }
        glyph += (i % zoom) ? 0 : FONT_BPL;
    }
}

void drawString(int x, int y, char *str, unsigned int attr, int zoom) {
    while (*str) {
        if (*str == '\r') {
            x = 0;
        } else if (*str == '\n') {
            x = 0;
            y += (FONT_HEIGHT * zoom);
        } else {
            drawChar(*str, x, y, attr, zoom);
            x += (FONT_WIDTH * zoom);
        }
        str++;
    }
}

/* Example: Show green HELLO WORLD text on the screen */
// drawString(0, 0, "HELLO WORLD !!!", 0x0000BB00, 1);



/* New function for Uart: Check and return if no new character, don't wait */

#if 1 //UART0

unsigned int uart_isReadByteReady() {
    return (!(UART0_FR & UART0_FR_RXFE));
}

#else //UART1

unsigned int uart_isReadByteReady() {
    return (AUX_MU_LSR & 0x01);
}

#endif

unsigned char getUart() {
    unsigned char ch = 0;
    if (uart_isReadByteReady())
        ch = uart_getc();
    return ch;
}


/* Functions to delay, set/wait timer */

void wait_msec(unsigned int msVal) {
    register unsigned long f, t, r, expiredTime; //64 bits

    // Get the current counter frequency (Hz), 1Hz = 1 pulses/second
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));

    // Read the current counter value
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));

    // Calculate expire value for counter
    /* Note: both expiredTime and counter value t are 64 bits,
    thus, it will still be correct when the counter is overflow */
    expiredTime = t + f * msVal / 1000;

    do {
        asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    } while (r < expiredTime);
}


void set_wait_timer(int set, unsigned int msVal) {
    static unsigned long expiredTime = 0; //declare static to keep value
    register unsigned long r, f, t;

    if (set) { /* SET TIMER */
        // Get the current counter frequency (Hz)
        asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));

        // Read the current counter
        asm volatile ("mrs %0, cntpct_el0" : "=r"(t));

        // Calculate expired time:
        expiredTime = t + f * msVal / 1000;
    } else { /* WAIT FOR TIMER TO EXPIRE */
        do {
            asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
        } while (r < expiredTime);
    }
}

// Compare two string
int strComp(char *str1, char *str2) {
    while (*str1 != '\0' && *str2 != '\0') {

        // strings not equal
        if (*str1 != *str2) {
            return 0;
        }
        str1++;
        str2++;
    }

    // return true if reach the end and no mismatched
    return (*str1 == '\0' && *str2 == '\0');
}

// Match until function to use in tab function
int startsWith(char *full, char *prefix) {
    while (*prefix != '\0') {

        // if missed match happened return false
        if (*prefix != *full) {
            return 0;
        }
        prefix++;
        full++;
    }

    // if all character in the prefix match the command
    return 1;
}

int is_all_digits(const char *str) {
    if (*str == '\0')
        return 0; // empty string not allowed
    while (*str) {
        if (*str < '0' || *str > '9')
            return 0;
        str++;
    }
    return 1;
}

int my_atoi(const char *str) {
    int result = 0;
    int i = 0;

    // Skip optional leading + sign
    if (str[0] == '+') {
        i++;
    }

    while (str[i] != '\0') {
        if (str[i] < '0' || str[i] > '9') {
            return -1; // Invalid input: non-digit character
        }
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return result;
}

void strCopy(char *dest, const char *src) {
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
}

int strLen(const char *str) {
    int len = 0;
    while (str[len])
        len++;
    return len;
}

void intToStr(int num, char *str) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers
    if (num < 0) {
        isNegative = 1;
        num = -num;
    }

    // Process digits in reverse
    while (num != 0) {
        int digit = num % 10;
        str[i++] = digit + '0';
        num /= 10;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    // Terminate string
    str[i] = '\0';

    // Reverse the string
    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = tmp;
    }
}

