#include "game.h"
#include "../drivers/uart0.h"
#include "../drivers/framebf.h"
#include "../assets/gameAssets.h"

// static gameStage = 1;

void gameMenu(){
    drawImg(title_start, 0, 0, 1024, 768);
    int isStart = 1;

    uart_puts("\n Enter Game Menu");
    // Loop for main game menu
    while (1){
        // Get user input
        char c = uart_getc();

        // Direction arrow will be replace by awsd button instead
        switch (c){
        case 'w':
            isStart = 1;
            drawImg(title_start, 0, 0, 1024, 768);
            // show the play game screen
            break;
        case 's':
            isStart = 0;
            drawImg(title_exit, 0, 0, 1024, 768);
            break;
        case '\n':
            if (isStart){
                uart_puts("\nGame enter");
                gameLoop();
            } else {
                uart_puts("\nGame exist");
                clearScreen();
                return;
            }
            break;
        default:
            break;
        }
    }
    
}

void gameLoop(){
    drawImg(stage1, 0, 0, 1024, 768);
    drawImg(basketball_web, 700, 600, 32, 24);
    while (1){
        char c = uart_getc();
        if (c == '\n'){
            return;
        }
    }
}