// Create command array type to store the command information
typedef struct {
    char* name;           // Name of the command
    char* des;            // Command description
    void(*cmdFunc)(void); // pointer to the actual function of the commands
} commandArr;


void cmdProcess(char* cmdBuff);
void help();
void helpC();
void clear();
void braudRate();
void handShake();