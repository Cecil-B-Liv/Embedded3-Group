#include "game.h"
#include "../drivers/uart0.h"
#include "../drivers/framebf.h"
#include "../assets/gameAssets.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

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
            drawGameBackGround(title_start);
            // show the play game screen
            break;
        case 's':
            isStart = 0;
            drawGameBackGround(title_exit);
            break;
        case '\n':
            if (isStart){
                uart_puts("\nGame enter");
                clearScreen();
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
    int start_x = 500;
    int start_y = 600;
    int player_height = 100;
    int player_width = 100;

    // Initialize the player
    GameObject player = {1, start_x, start_y, player_height, player_width, 1, basketball_hoops};

    drawGameBackGround(stage1);
    drawObject(&player);

    while (1){
        char c = uart_getc();
        if (c == '\n'){
            //drawGameBackGround(title_start);
            eraseObject(&player, stage1);
            return;
        }
    }
}

// Function to draw the game background
void drawGameBackGround(const unsigned int* bg){
    drawImg(bg, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void drawObject(GameObject *obj){
    drawImg(obj->sprite, obj->y, obj->x, obj->width, obj->height);
}

void moveObject(GameObject *obj, int direct){
    // Earase the object function here

    // Validate the value
    obj->x += direct;

    if (obj->x < 0) obj->x = 0; // if touch the left boarder
    if (obj->x + obj->width > SCREEN_WIDTH) obj->x = SCREEN_WIDTH; // if touch the right  border

    // Redraw new object
    drawObject(obj);
}

// Fill the old object with the respected background area
void eraseObject(GameObject *obj, const unsigned int *bg) {
    for (int i = 0; i < obj->height; i++) {
        for (int j = 0; j < obj->width; j++) {
            int x = obj->x + j;
            int y = obj->y + i;
            unsigned int pixel = bg[(y * SCREEN_WIDTH) + x];
            drawPixelARGB32(x, y, pixel);
        }
    }
}
