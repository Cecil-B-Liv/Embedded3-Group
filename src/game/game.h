typedef struct {
    int type;
    int x, y;
    int width, height;
    int speed;
    int alive;
    const unsigned int *sprite;
} GameObject;

void gameLoop();
void moveObject(GameObject *obj, int dx, int dy);
void drawObject(GameObject *obj);
void eraseObject(GameObject *obj);

void gameMenu();
void drawGameBackGround(const unsigned int* bg);

void resetPlayer();
void changeToStage(const unsigned int* stage);

void spawnBall();
void updateBalls();

void checkCollision();