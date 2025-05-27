#include "game.h"

#include "../assets/gameAssets.h"
#include "../drivers/framebf.h"
#include "../drivers/uart0.h"
#include "../util/utils.h"

// System Timer Counter Lower 32 bits
#define SYS_TIMER_CLO (*(volatile unsigned int *)(MMIO_BASE + 0x00003004))

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define PLAYER_START_X 412
#define PLAYER_START_Y 568
#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT 100
#define PLAYER_SPEED 10
#define MAX_OBJECTS 20

#define BALL_WIDTH 50
#define BALL_HEIGHT 50
#define BALL_SPEED 5
#define MAX_BALLS 10

#define NORMAL_SCORE 10
#define SPECIAL_SCORE 30
#define BOMB_SCORE -100
#define SCORE_MULTIPLIER 2

#define PLAYER_TAG 1
#define NORMAL_BALL_TAG 2
#define SPEICAL_BALL_TAG 3
#define BOMB_TAG 4
#define ENLARGE_TAG 5
#define SCORE_MULTIPLY_TAG 6

#define ENLARGE_TIME 10
#define MULTIPLY_TIME 10

#define STAGE1_SCORE 60
#define STAGE2_SCORE 60
#define STAGE3_SCORE 60

#define STAGE1_TIME 30
#define STAGE2_TIME 40
#define STAGE3_TIME 60

#define BASE_SCORE_MULTIPLIER 1
#define OBJECT_LOCATION_Y 65

#define MENU_TAG_PAUSE 0
#define MENU_TAG_LOSE 1

#define STATUSBAR_Y_START 0
#define STATUSBAR_HEIGHT 60

static volatile GameObject player = {.type = PLAYER_TAG,
                                     .x = PLAYER_START_X,
                                     .y = PLAYER_START_Y,
                                     .height = PLAYER_HEIGHT,
                                     .width = PLAYER_WIDTH,
                                     .speed = PLAYER_SPEED,
                                     .alive = 1,
                                     .sprite = basketball_hoops}; // Player object

static int score = 0;
static int frameCount = 0;            // frames count, use as base for all timer
static int timerCount = STAGE1_TIME;  // second

static int x2ScoreTimeCount = 0;      // Timer for x2 score multiplier item
static int x2ScoreActive = 0;         // flag of x2 score multiplier item

static int enlargeTimeCount = 0;      // Timer for x2 size item
static int enlargeActive = 0;         // flag of x2 size item

static int breakGameLoop = 0;         // flag to indicate breaking the game loop and go back to the main menu

static int current_stage_index = 0;   // index of the current stage in game loop
const unsigned long *stages[] = {stage1, stage2, stage3}; // array to store all game stage
static const unsigned long *current_stage = stage1;       // set the current stage to stage 1 as default, change latter when game proceed 

static volatile GameObject objects[MAX_OBJECTS];          // Player object, using volatitle so that the meomory region will not be touch during software life time
// static gameStage = 1;

// Game menu loop, use as welcome screen, enter or exit the game
void gameMenu() {

    // Draw the main menu screen
    drawGameBackGround(title_start);
    int isStart = 1; // flag to check if the game is start

    uart_puts("\nEnter Game Menu");
    // Loop for main game menu
    while (1) {
        // Get user input
        char c = uart_getc();

        // Direction arrow will be replace by awsd button instead
        switch (c) {
            case 'w': // Player current choice is play the game
                isStart = 1;
                drawGameBackGround(title_start);
                // show the play game screen
                break;
            case 's': // Player current choice is exit the game
                isStart = 0;
                drawGameBackGround(title_exit);
                break;
            case '\n': // Player confrim choice
                if (isStart) { // enter the game
                    uart_puts("\nGame enter");
                    clearScreen();
                    gameLoop();

                } else { // exit the game
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

// main game loop, handle all logic during the gameplay
void gameLoop() {

    // Initialize the game, drawing the current stage (default stage 1), and draw the player at starting position
    drawGameBackGround(current_stage);
    drawObject(&player);
    
    // Loop to handle game logic
    while (1) {
        // If the flag is set, reset everything and go back to the main menu
        if (breakGameLoop) {
            breakGameLoop = 0;  // reset the flag
            current_stage_index = 0; // reset the stage index
            changeToStage(stages[current_stage_index]);  // change to the first
                                                         // stage again
            drawGameBackGround(title_start);             // main menu
            return;
        }

        // Refresh the status bar on top of the screen that display time, goal and current score after each loop
        refreshStatusBar();

        updateBalls(); // Update the falling game objects
        checkCollision(); // checking the collision between player and all objects in the game

        checkStageProgression(); // Checking the condition of wining stage, losing stage using score

        // Cacluate the time based on the current frames count
        if (frameCount % 30 == 0) { // game run of 30 fps meaning 30 frames = 1 second
            uart_puts("\nTime Left: ");
            uart_dec(timerCount);
            timerCount--;

            // If flag of x2 score and x2 size is activate, calculate the timer of them
            if (x2ScoreActive) {
                x2ScoreTimeCount++;
            }
            if (enlargeActive) {
                enlargeTimeCount++;
            }
        }

        // Check if the counter is expired for x2 score then reset both the flag and the timer
        if (x2ScoreTimeCount == MULTIPLY_TIME) {
            x2ScoreActive = 0;
            x2ScoreTimeCount = 0;
        }

        // Check if the counter is expired for x2 size then reset both the flag and the timer
        if (enlargeTimeCount == ENLARGE_TIME) {

            // Remove the enlarged player, change the size and sprite back to origin size then draw again
            eraseObject(&player);
            player.sprite = basketball_hoops;
            player.width = PLAYER_WIDTH;
            drawObject(&player);

            enlargeActive = 0;
            enlargeTimeCount = 0;
        }

        // Time limit reached, game over
        if (timerCount == 0) {
            uart_puts("\nTime limit reached, game lose");

            // Enter the losing menu to let player choose either to reset the current stage or exit the game
            if (inGameMenuChoice(MENU_TAG_LOSE)) { // if player want to exit the game
                breakGameLoop = 1;
                continue;
            } else {  // if player want to rest the stage
                breakGameLoop = 0;
                resetGameObjects();  // Reset all game objects
                drawGameBackGround(current_stage); // redraw the current stage
                drawObject(&player); // redraw the player at starting position
                continue;
            }
        }

        // spawn object every 60 frames (2s)
        if (frameCount == 60) {
            spawnBall();
            frameCount = 0;
        }

        // User input handling 
        if (uart_is_read_ready()) {
            char c = uart_getc();

            switch (c) {
                case 'a': // go left
                    moveObject(&player, -1, 0);
                    break;
                case 'd': // go right
                    moveObject(&player, +1, 0);
                    break;
                case 27: // escape, pausing the game, user can exit the game or continue the game, all game object state are keeped if continue
                    if (inGameMenuChoice(MENU_TAG_PAUSE)) {
                        breakGameLoop = 1;
                    } else { // continue the game
                        breakGameLoop = 0;

                        // Redraw every thing back to the state before pausing
                        drawGameBackGround(current_stage);
                        drawObject(&player);

                        // Redraw all falling objects
                        for (int i = 0; i < MAX_OBJECTS; i++) {
                            if (objects[i].alive) {
                                drawObject(&objects[i]);
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        // Update frame count and wait for 33ms (lock the game at 30fps)
        frameCount++;
        wait_msec(33);
    }
}

// Function to get the random falling object tags for ball spawing,
int getRandomBallType(int stage) {
    int r = (SYS_TIMER_CLO + score) % 100; // randomness based on the lower system count 32 bit and current score of the player
    if (stage == 0) {
        // Stage 1 spawn rates:
        // 85% Normal, 7% Special, 4% Enlarge, 4% x2 Score
        if (r < 85)
            return NORMAL_BALL_TAG;  // 0–84 (85%)
        else if (r < 92)
            return SPEICAL_BALL_TAG;  // 85–91 (7%)
        else if (r < 96)
            return ENLARGE_TAG;  // 92–95 (4%)
        else
            return SCORE_MULTIPLY_TAG;  // 96–99 (4%)
    } else if (stage == 1) {
        // Stage 2 spawn rates:
        // 65% Normal, 15% Special, 10% Bomb, 5% Enlarge, 5% x2 Score
        if (r < 65)
            return NORMAL_BALL_TAG;  // 0–64 (65%)
        else if (r < 80)
            return SPEICAL_BALL_TAG;  // 65–79 (15%)
        else if (r < 90)
            return BOMB_TAG;  // 80–89 (10%)
        else if (r < 95)
            return ENLARGE_TAG;  // 90–94 (5%)
        else
            return SCORE_MULTIPLY_TAG;  // 95–99 (5%)
    } else {
        // Stage 3 spawn rates:
        // 55% Normal, 15% Special, 15% Bomb, 7% Enlarge, 8% x2 Score
        if (r < 55)
            return NORMAL_BALL_TAG;  // 0–54 (55%)
        else if (r < 70)
            return SPEICAL_BALL_TAG;  // 55–69 (15%)
        else if (r < 85)
            return BOMB_TAG;  // 70–84 (15%)
        else if (r < 92)
            return ENLARGE_TAG;  // 85–91 (7%)
        else
            return SCORE_MULTIPLY_TAG;  // 92–99 (8%)
    }

    // return normal ball as default
    return 0;
}

// Function to check the score of current stage and condition checking for passing level
void checkStageProgression() {

    // If the score is hit at negative -100 threadhold (hit the bomb when having 0 score) then losing the game
    if (score <= -100) {
        uart_puts("\nNegative score threedhold reach, lose game");
        if (inGameMenuChoice(MENU_TAG_LOSE)) { // enter the losing menu to let player decide to continue game or exit the game
            breakGameLoop = 1;
            return;
        } else {  // Continue Play the game
            breakGameLoop = 0;
            resetGameObjects();  // Reset all game objects
            drawGameBackGround(current_stage); // redraw the current stage
            drawObject(&player); // redraw player at starting position
            return;
        }
    }

    // Checking score to pass for every level
    if (score >= STAGE1_SCORE && current_stage_index == 0) {
        current_stage_index = 1; // change index to stage 2
        timerCount = STAGE2_SCORE; // set the time limit to stage 2 time limit
        changeToStage(stages[current_stage_index]); // change current stage to stage 2
        drawGameBackGround(current_stage); // draw the stage 2 background
        drawObject(&player); // draw player at starting positioin
    } else if (score >= STAGE2_SCORE && current_stage_index == 1) {
        current_stage_index = 2; // change index to stage 2
        timerCount = STAGE3_SCORE; // set the time limit to stage 3 limit
        changeToStage(stages[current_stage_index]); // change the current stage to 3
        drawGameBackGround(current_stage); // draw the stage 3
        drawObject(&player); // draw player at starting position
    } else if (score >= STAGE3_SCORE && current_stage_index == 2) { // win the stage 3 meaning wining the game
        breakGameLoop = 1; // break the game loop flag set
        drawGameBackGround(win_game_menu); // wining menu display
        // wait for player to confirm the winning menu
        while (1) {
            char c = uart_getc();
            if (c == 'x' || c == '\n') break;
        }
    }
}

// Colistion checking between the player and the game objects
void checkCollision() {
    // TEMP VARRIABLE WILL BE REMOVE IN FINAL PRODUCT
    int previousScore = score;
    // TEMP VARRIABLE WILL BE REMOVE IN FINAL PRODUCT
    int multiplier = x2ScoreActive ? 2 : 1; // check if the x2 score flag is set then set the multiplier accordingly 

    // loop through all the game object for collision checking
    for (int i = 0; i < MAX_BALLS; i++) {
        // only check if the ball is alive
        if (!objects[i].alive) continue;

        // collision checking
        if (player.x < objects[i].x + objects[i].width &&
            player.x + player.width > objects[i].x &&
            player.y < objects[i].y + objects[i].height &&
            player.y + player.height > objects[i].y) {
            objects[i].alive = 0;      // mark as caught
            eraseObject(&objects[i]);  // visually remove

            // score checking
            // normal ball
            if (objects[i].type == NORMAL_BALL_TAG) {
                score += NORMAL_SCORE * multiplier;
                continue;
            }
            // Special ball
            if (objects[i].type == SPEICAL_BALL_TAG) {
                score += SPECIAL_SCORE * multiplier;
                continue;
            }
            // Bomb
            if (objects[i].type == BOMB_TAG) {
                score += BOMB_SCORE;
                continue;
            }
            // enlarge
            if (objects[i].type == ENLARGE_TAG) {

                // set the player sprite and player width to the enlarged one
                player.sprite = basketball_hoops_large, player.width = 160;

                // earase and redraw the player to change to larger one
                eraseObject(&player);
                drawObject(&player);

                enlargeActive = 1; // set the enlarger flag
            }
            // multipler
            if (objects[i].type == SCORE_MULTIPLY_TAG) {
                x2ScoreActive = 1; // set the x2 score flag
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
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!objects[i].alive) {
            // Set up the ball

            // Get the random object
            int ball_type = getRandomBallType(current_stage_index);

            // Get the sprite of the object
            const unsigned long *sprite = normal_ball;  // default value
            if (ball_type == SPEICAL_BALL_TAG)
                sprite = special_ball;
            else if (ball_type == BOMB_TAG)
                sprite = bomb;
            else if (ball_type == ENLARGE_TAG)
                sprite = x2_size;
            else if (ball_type == SCORE_MULTIPLY_TAG)
                sprite = x2_score;

            // set up the game object according to the value get from the get random ball
            objects[i] = (GameObject){
                .type = ball_type,
                .x = SYS_TIMER_CLO % (SCREEN_WIDTH + score - BALL_WIDTH),  // use system counter and score as random factor to spawn 
                .y = OBJECT_LOCATION_Y,
                .width = BALL_WIDTH,
                .height = (ball_type == SPEICAL_BALL_TAG) ? 80 : BALL_HEIGHT,
                .speed = BALL_SPEED,
                .alive = 1,
                .sprite = sprite};

            // draw the object on the screen
            drawObject(&objects[i]);
            break;
        }
    }
}

// Update the ball position, called every game loop to move the ball
void updateBalls() {

    // only move the alive (on screen) ball
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!objects[i].alive) continue;

        // Erase before moving
        eraseObject(&objects[i]);

        // Move the object
        objects[i].y += objects[i].speed;

        // If ball reaches the bottom, mark as not alive and erase
        // Bottom will count as the foot of the player
        if (objects[i].y + objects[i].height >=
            PLAYER_START_Y + PLAYER_HEIGHT) {
            objects[i].alive = 0;
            eraseObject(&objects[i]);
            continue;
        }

        // draw object at new position
        drawObject(&objects[i]);
    }
}

// Reset all game object
void resetGameObjects() {
    // Reset Player position
    player.x = PLAYER_START_X;
    player.y = PLAYER_START_Y;
    uart_puts("\nReset Player Position and Score");

    // reset the time limit depend on current stage
    if (current_stage_index == 0)
        timerCount = STAGE1_TIME;
    else if (current_stage_index == 1)
        timerCount = STAGE2_TIME;
    else
        timerCount = STAGE3_TIME;

    // reset the frame count and score
    frameCount = 0;
    score = 0;

    // resest the flag, timer, player size for the enlarge feature
    enlargeActive = 0;
    enlargeTimeCount = 0;
    eraseObject(&player);
    player.sprite = basketball_hoops;
    player.width = PLAYER_WIDTH;
    drawObject(&player);

    // reset the flag and timer for x2 score
    x2ScoreActive = 0;
    x2ScoreTimeCount = 0;

    // Reset game objets
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i] = (GameObject){0};
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

// draw the game object on screen
void drawObject(volatile GameObject *obj) {
    drawImg(obj->sprite, obj->y, obj->x, obj->width, obj->height, 1);
}

// move the game object
void moveObject(volatile GameObject *obj, int dx, int dy) {

    // erase before moving
    eraseObject(obj);
    obj->x += obj->speed * dx;
    obj->y += obj->speed * dy;

    // move the object
    if (obj->x < 0) obj->x = 0;
    if (obj->x + obj->width > SCREEN_WIDTH) obj->x = SCREEN_WIDTH - obj->width;
    if (obj->y < 0) obj->y = 0;
    if (obj->y + obj->height > SCREEN_HEIGHT)
        obj->y = SCREEN_HEIGHT - obj->height;

    // draw object at new position
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

// Display losing, pausing menu and handle user input
int inGameMenuChoice(int menu_tag) {
    int choice = 0;  // 0 mean continue 1 mean exit
    // Draw the initial screen
    if (menu_tag == MENU_TAG_PAUSE) {
        drawGameBackGround(pause_continue_menu);
    } else if (menu_tag == MENU_TAG_LOSE) {
        drawGameBackGround(lose_continue_menu);
    }

    while (1) {
        char c = uart_getc();

        switch (c) {
            case 'w':
                choice = 0;
                if (menu_tag == MENU_TAG_PAUSE)
                    drawGameBackGround(pause_continue_menu);
                else
                    drawGameBackGround(lose_continue_menu);
                break;

            case 's':
                choice = 1;
                if (menu_tag == MENU_TAG_PAUSE)
                    drawGameBackGround(pause_exit_menu);
                else
                    drawGameBackGround(lose_exit_menu);
                break;

            case '\n':
                return choice;

            default:
                break;
        }
    }
}

// refresh the status bar at the top of the screen (time, goal, socre)
void refreshStatusBar() {
    clearStatusBar(); // clear status bar before display new bar
    drawStatusBar();  // draw new bar according to the current value
}

// clear the bar and fill back with the respected background area
void clearStatusBar() {
    for (int y = STATUSBAR_Y_START; y < STATUSBAR_Y_START + STATUSBAR_HEIGHT;
         y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            unsigned int pixel = current_stage[(y * SCREEN_WIDTH) + x];
            drawPixelARGB32(x, y, pixel);
        }
    }
}

// draw the status bar
void drawStatusBar() {
    // String to store value
    char timeStr[12];
    char goalStr[12];
    char scoreStr[12];

    // convert number to string
    intToStr(timerCount, timeStr);
    intToStr(STAGE1_SCORE, goalStr);
    intToStr(score, scoreStr);

    if (current_stage_index == 0)
        intToStr(STAGE1_SCORE, goalStr);
    else if (current_stage_index == 1)
        intToStr(STAGE2_SCORE, goalStr);
    else
        intToStr(STAGE3_SCORE, goalStr);

    drawString(40, 5, "Time:", 0, 4);
    drawString(200, 5, timeStr, 0, 4);

    drawString(388, 5, "Goal:", 0, 4);
    drawString(550, 5, goalStr, 0, 4);

    drawString(716, 5, "Score:", 0, 4);
    drawString(901, 5, scoreStr, 0, 4);
}
