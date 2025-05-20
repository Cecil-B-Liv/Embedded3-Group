#include "stringUltil.h"

// Compare two string
int strComp(char *str1, char* str2){
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
int startsWith(char* full, char* prefix) {
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