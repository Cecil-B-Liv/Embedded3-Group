#include "game.h"
#include "../drivers/uart0.h"
#include "../drivers/framebf.h"
#include "../assets/gameAssets.h"
#include "../util/utilsSap.h"

#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768

#define PLAYER_START_X  500
#define PLAYER_START_Y  600
#define PLAYER_WIDTH    100
#define PLAYER_HEIGHT   100
#define PLAYER_SPEED    10
#define MAX_BALLS       10

#define BALL_WIDTH      50
#define BALL_HEIGHT     50
#define BALL_SPEED      5
#define MAX_BALLS       10

static GameObject player = {.type = 1,
                            .x = PLAYER_START_X, 
                            .y = PLAYER_START_Y, 
                            .height = PLAYER_HEIGHT, 
                            .width = PLAYER_WIDTH, 
                            .speed = PLAYER_SPEED, 
                            .alive = 1,
                            .sprite = basketball_hoops
};
static int score = 0;
static unsigned int *current_stage = stage1;

static GameObject balls[MAX_BALLS];

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
    
    int frameCount = 0;
    while (1){
        
        if (uart_is_read_ready()){
            char c = uart_getc();
        
            switch (c){
            case 'a':
                moveObject(&player, -1, 0);
                break;
            case 'd':
                moveObject(&player, +1, 0);
                break;

            default:
                break;
            }
        }
        
        if (frameCount % 60 == 0) { // spawn object every 60 frames
            spawnBall();
            frameCount = 0;
        }

        // Move the balls
        updateBalls();
        
        // Update frame count and wait
        frameCount++;
        wait_msec(33);
    }
}

// spawn the ball from the array
void spawnBall() {
    // If one avaiable ball from the array
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].alive) {

            // Set up the ball
            balls[i] = (GameObject){
                .type = 2,
                .x = (i * 100) % (SCREEN_WIDTH - BALL_WIDTH),
                .y = 0,
                .width = BALL_WIDTH,
                .height = BALL_HEIGHT,
                .speed = BALL_SPEED,
                .alive = 1,
                .sprite = normal_ball
            };
            drawObject(&balls[i]);
            break;
        }
    }
}

// Update the ball position
void updateBalls() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].alive) continue;

        // Erase before moving
        eraseObject(&balls[i]);
        balls[i].y += balls[i].speed;

        // If ball reaches the bottom, mark as not alive and erase
        if (balls[i].y + balls[i].height >= SCREEN_HEIGHT) {
            balls[i].alive = 0;
            eraseObject(&balls[i]);
            continue;
        }

        drawObject(&balls[i]);
    }
}

// Reset the player position
void resetPlayer(){
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;

    score = 0;
    uart_puts("\n Reset Player Position and Score");
}

// change the desired stage
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

void moveObject(GameObject *obj, int dx, int dy){
    eraseObject(obj);
    obj->x += obj->speed * dx;
    obj->y += obj->speed * dy;

    if (obj->x < 0) obj->x = 0;
    if (obj->x + obj->width > SCREEN_WIDTH) obj->x = SCREEN_WIDTH - obj->width;
    if (obj->y < 0) obj->y = 0;
    if (obj->y + obj->height > SCREEN_HEIGHT) obj->y = SCREEN_HEIGHT - obj->height;

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
