#include "game.h"
#include "../drivers/uart0.h"
#include "../drivers/framebf.h"
#include "../assets/gameAssets.h"

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768

#define PLAYER_START_X  500
#define PLAYER_START_Y  600
#define PLAYER_WIDTH    100
#define PLAYER_HEIGHT   100
#define PLAYER_SPEED    10

static GameObject player = {1, PLAYER_START_X, PLAYER_START_Y, PLAYER_HEIGHT , PLAYER_WIDTH, PLAYER_SPEED, 1, basketball_hoops};
static int score = 0;
static unsigned int *current_stage = stage1;

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
    drawGameBackGround(current_stage);
    drawObject(&player);
    
    while (1){
        char c = uart_getc();
        
        switch (c){
        case 'a':
            moveObject(&player, -1);
            break;
        case 'd':
            moveObject(&player, +1);
            break;

        default:
            break;
        }
    }
}

void resetPlayer(){
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;

    score = 0;
    uart_puts("\n Reset Player Position and Score");
}

void changeToStage(const unsigned int* stage){
    current_stage = stage;

    resetPlayer();
}

// Function to draw the game background
void drawGameBackGround(const unsigned int* bg){
    drawImg(bg, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void drawObject(GameObject *obj){
    drawImg(obj->sprite, obj->y, obj->x, obj->width, obj->height);
}

void moveObject(GameObject *obj, int direct){
    eraseObject(obj);

    // Validate the value
    obj->x += obj->speed * direct;

    if (obj->x < 0) obj->x = 0; // if touch the left boarder
    if (obj->x + obj->width > SCREEN_WIDTH) obj->x = SCREEN_WIDTH; // if touch the right  border

    // Redraw new object
    drawObject(obj);
}

// Fill the old object with the respected background area
void eraseObject(GameObject *obj) {
    for (int i = 0; i < obj->height; i++) {
        for (int j = 0; j < obj->width; j++) {
            int x = obj->x + j;
            int y = obj->y + i;
            unsigned int pixel = current_stage[(y * SCREEN_WIDTH) + x];
            drawPixelARGB32(x, y, pixel);
        }
    }
}
