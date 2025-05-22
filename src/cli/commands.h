// Create command array type to store the command information
typedef struct {
    char *name;           // Name of the command
    char *des;            // Command description
    void (*cmdFunc)(char *args); // pointer to the actual function of the commands, take the argument of each function
} commandArr;


void cmdProcess(char *cmdBuff);

void help(char *arg);

void clear(char *arg);

void baudRate(char *arg);

void handShake(char *arg);

void showInfo(char *arg);

char *getBoardModel(int rev);

void error(char *error);

void teamDisplay();
void videoDisplay(char *arg);

void getBaud();
