#include "stringUtil.h"

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
    if (*str == '\0') return 0; // empty string not allowed
    while (*str) {
        if (*str < '0' || *str > '9') return 0;
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
            return -1;  // Invalid input: non-digit character
        }
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return result;
}