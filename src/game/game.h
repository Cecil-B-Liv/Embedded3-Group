typedef struct {
    int type;
    int x, y;
    int width, height;
    int speed;
    int alive;
    const unsigned long* sprite;
} GameObject;

typedef struct {
    int tag;
    int probability;
} BallProbability;
void gameLoop();
void moveObject(volatile GameObject* obj, int dx, int dy);
void drawObject(volatile GameObject* obj);
void eraseObject(volatile GameObject* obj);

void gameMenu();
void drawGameBackGround(const unsigned long* bg);

void changeToStage(const unsigned long* stage);

void spawnBall();
void updateBalls();

void checkCollision();

void checkStageProgression();
int getRandomBallType(int stage);

void resetGameObjects();
int inGameMenuChoice(int menu_tag);

void refreshStatusBar();
void clearStatusBar();
void drawStatusBar();