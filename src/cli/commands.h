// Create command array type to store the command information
typedef struct {
    char* name;           // Name of the command
    char* des;            // Command description
    void(*cmdFunc)(char *args); // pointer to the actual function of the commands, take the argument of each function
} commandArr;


void cmdProcess(char* cmdBuff);
void help();
void helpC();
void clear();
void braudRate();
void handShake();

int getArgIndex(char* commandBuff);