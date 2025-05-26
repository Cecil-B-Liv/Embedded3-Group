#include "game.h"
#include "../drivers/uart0.h"
#include "../drivers/framebf.h"
#include "../assets/gameAssets.h"
#include "../util/utils.h"

//System Timer Counter Lower 32 bits
#define SYS_TIMER_CLO  (* (volatile unsigned int*)(MMIO_BASE + 0x00003004))


#define SCREEN_WIDTH           1024
#define SCREEN_HEIGHT          768

#define PLAYER_START_X         412
#define PLAYER_START_Y         568
#define PLAYER_WIDTH           80
#define PLAYER_HEIGHT          100
#define PLAYER_SPEED           10
#define MAX_OBJECTS            20

#define BALL_WIDTH             50
#define BALL_HEIGHT            50
#define BALL_SPEED             5
#define MAX_BALLS              10

#define NORMAL_SCORE           10
#define SPECIAL_SCORE          30
#define BOMB_SCORE            -100
#define BASE_MULTIPLIER        1

#define PLAYER_TAG             1
#define NORMAL_BALL_TAG        2
#define SPEICAL_BALL_TAG       3
#define BOMB_TAG               4
#define ENLARGE_TAG            5
#define SCORE_MULTIPLY_TAG     6

#define STAGE1_SCORE           60
#define STAGE2_SCORE           60
#define STAGE3_SCORE           60

#define STAGE1_TIME            30
#define STAGE2_TIME            40
#define STAGE3_TIME            60

#define BASE_SCORE_MULTIPLIER  1
#define OBJECT_LOCATION_Y      65

static volatile GameObject player = {.type = PLAYER_TAG,
        .x = PLAYER_START_X,
        .y = PLAYER_START_Y,
        .height = PLAYER_HEIGHT,
        .width = PLAYER_WIDTH,
        .speed = PLAYER_SPEED,
        .alive = 1,
        .sprite = basketball_hoops
};

static int score = 0;
static int end = 0;

static int current_stage_index = 0;
const unsigned long *stages[] = {stage1, stage2, stage3};
static const unsigned long *current_stage = stage1;

static volatile GameObject objects[MAX_OBJECTS];
// static gameStage = 1;

void gameMenu() {
    drawGameBackGround(title_start);
    int isStart = 1;

    uart_puts("\n Enter Game Menu");
    // Loop for main game menu
    while (1) {
        // Get user input
        char c = uart_getc();

        // Direction arrow will be replace by awsd button instead
        switch (c) {
            case 'w':isStart = 1;
                drawGameBackGround(title_start);
                // show the play game screen
                break;
            case 's':isStart = 0;
                drawGameBackGround(title_exit);
                break;
            case '\n':
                if (isStart) {
                    uart_puts("\nGame enter");
                    clearScreen();
                    gameLoop();

                } else {
                    uart_puts("\nGame exist");
                    clearScreen();
                    return;
                }
                break;
            default:break;
        }
    }

}

void gameLoop() {
    drawGameBackGround(current_stage);
    drawObject(&player);

    int frameCount = 0; // frames
    int timerCount = 0; // second
    while (1) {
        if (end) {
            score = 0; // reset the score
            end = 0;   // reset the game end flag
            current_stage_index = 0;
            resetGameObjects();
            changeToStage(stages[0]); // change to the first stage again
            drawGameBackGround(title_start); // main menu
            return;
        }
        // Move the balls
        updateBalls();
        checkCollision();

        checkStageProgression();

        // timer
        if (frameCount % 30 == 0) {
            uart_puts("\nCurrent time eslap: ");
            uart_dec(timerCount);
            timerCount++;
        }

        // Time limit reach then end game
        if (checkTimeLimit(timerCount)) {
            uart_puts("\nTime limit reach, game lose");
            end = 1;
            continue;
        }

        // spawn object every 60 frames (2s)
        if (frameCount == 60) {
            spawnBall();
            frameCount = 0;
        }

        if (uart_is_read_ready()) {
            char c = uart_getc();

            switch (c) {
                case 'a':moveObject(&player, -1, 0);
                    break;
                case 'd':moveObject(&player, +1, 0);
                    break;

                default:break;
            }
        }

        // Update frame count and wait
        frameCount++;
        wait_msec(33);
    }
}

int getRandomBallType(int stage) {
    int r = (SYS_TIMER_CLO + score) % 100;
    // Stage 1
    if (stage == 0) {
        if (r < 90) return NORMAL_BALL_TAG; // 90% rate
        else if (r < 99) return SPEICAL_BALL_TAG; // 10% rate
        else return NORMAL_BALL_TAG;  // <- fallback if r >= 99
        // Stage 2
    } else if (stage == 1) {
        if (r < 70) return NORMAL_BALL_TAG; // 70% rate
        else if (r < 90) return SPEICAL_BALL_TAG; // 20% rate
        else return BOMB_TAG; // 10% rate
        // Stage 3
    } else {
        if (r < 60) return NORMAL_BALL_TAG; //60% rate
        else if (r < 80) return SPEICAL_BALL_TAG; // 20% rate
        else return BOMB_TAG; // 20% rate
    }

    // return normal ball as default
    return 0;
}

int checkTimeLimit(int timeCount) {
    if (current_stage_index == 0 && timeCount >= STAGE1_TIME) return 1;
    if (current_stage_index == 1 && timeCount >= STAGE2_TIME) return 1;
    if (current_stage_index == 2 && timeCount >= STAGE3_TIME) return 1;
    return 0;
}

void checkStageProgression() {
    if (score <= -100) {
        end = 1;
        uart_puts("\nNegative score threedhold reach, lose game");
        return;
    }

    if (score >= STAGE1_SCORE && current_stage_index == 0) {
        current_stage_index = 1;
        changeToStage(stages[1]);
        drawGameBackGround(current_stage);
        drawObject(&player);
    } else if (score >= STAGE2_SCORE && current_stage_index == 1) {
        current_stage_index = 2;
        changeToStage(stages[2]);
        drawGameBackGround(current_stage);
        drawObject(&player);
    } else if (score >= STAGE3_SCORE && current_stage_index == 2) {
        end = 1;
        uart_puts("\nYou win! Returning to menu...\n");
    }
}

void checkCollision() {
    // TEMP VARRIABLE WILL BE REMOVE IN FINAL PRODUCT
    int previousScore = score;
    // TEMP VARRIABLE WILL BE REMOVE IN FINAL PRODUCT
    for (int i = 1; i < MAX_BALLS; i++) {
        // only check if the ball is alive
        if (!objects[i].alive) continue;

        // collision checking
        if (player.x < objects[i].x + objects[i].width &&
            player.x + player.width > objects[i].x &&
            player.y < objects[i].y + objects[i].height &&
            player.y + player.height > objects[i].y) {

            objects[i].alive = 0;           // mark as caught
            eraseObject(&objects[i]);      // visually remove

            // score checking
            // normal ball
            if (objects[i].type == NORMAL_BALL_TAG) {
                score += NORMAL_SCORE;
                continue;
            }
            // Special ball
            if (objects[i].type == SPEICAL_BALL_TAG) {
                score += SPECIAL_SCORE;
                continue;
            }
            // Bomb
            if (objects[i].type == BOMB_TAG) {
                score += BOMB_SCORE;
                continue;
            }
        }
    }

    // TEMP VARRIABLE WILL BE REMOVE IN FINAL PRODUCT
    if (score != previousScore) {
        uart_puts("\nScore: ");
        uart_dec(score);
    }
    // TEMP VARRIABLE WILL BE REMOVE IN FINAL PRODUCT
}

// spawn the ball from the array
void spawnBall() {
    // If one avaiable ball from the array
    for (int i = 1; i < MAX_BALLS; i++) {
        if (!objects[i].alive) {
            // Set up the ball

            // Get the random object
            int ball_type = getRandomBallType(current_stage_index);

            // Get the sprite of the object
            const unsigned long *sprite = normal_ball; // default value
            if (ball_type == SPEICAL_BALL_TAG) sprite = special_ball;
            else if (ball_type == BOMB_TAG) sprite = bomb;

            objects[i] = (GameObject){
                .type = ball_type,
                .x = SYS_TIMER_CLO % (SCREEN_WIDTH + score - BALL_WIDTH), // use system counter and score as random seed
                .y = OBJECT_LOCATION_Y,
                .width = BALL_WIDTH,
                .height = (ball_type == SPEICAL_BALL_TAG) ? 80 : BALL_HEIGHT,
                .speed = BALL_SPEED,
                .alive = 1,
                .sprite = sprite
            };
            drawObject(&objects[i]);
            break;
        }
    }
}

// Update the ball position
void updateBalls() {
    for (int i = 1; i < MAX_BALLS; i++) {
        if (!objects[i].alive) continue;

        // Erase before moving
        eraseObject(&objects[i]);
        objects[i].y += objects[i].speed;

        // If ball reaches the bottom, mark as not alive and erase
        // Bottom will count as the foot of the player
        if (objects[i].y + objects[i].height >= PLAYER_START_Y + PLAYER_HEIGHT) {
            objects[i].alive = 0;
            eraseObject(&objects[i]);
            continue;
        }

        drawObject(&objects[i]);
    }
}

// Reset all game object
void resetGameObjects() {

    // Reset Player
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    uart_puts("\nReset Player Position and Score");
    score = 0;

    // Reset game objets
    for (int i = 1; i < MAX_OBJECTS; i++) {
        objects[i] = (GameObject) {0};
    }

    uart_puts("\nGame Objects Reset");
}

// change the desired stage
void changeToStage(const unsigned long *stage) {
    current_stage = stage;
    score = 0;

    // reset player and all game objects
    resetGameObjects();
}

// Function to draw the game background
void drawGameBackGround(const unsigned long *bg) {
    drawImg(bg, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

void drawObject(volatile GameObject *obj) {
    drawImg(obj->sprite, obj->y, obj->x, obj->width, obj->height, 1);
}

void moveObject(volatile GameObject *obj, int dx, int dy) {
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
void eraseObject(volatile GameObject *obj) {
    for (int i = 0; i < obj->height; i++) {
        for (int j = 0; j < obj->width; j++) {
            int x = obj->x + j;
            int y = obj->y + i;
            unsigned int pixel = current_stage[(y * SCREEN_WIDTH) + x];
            drawPixelARGB32(x, y, pixel);
        }
    }
}

void drawBarStatus(int currentTime, int timeLimit, int goal, int score){
    
}