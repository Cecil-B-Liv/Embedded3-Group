void drawChar(unsigned char ch, int x, int y, unsigned int attr, int zoom);

void drawString(int x, int y, char *str, unsigned int attr, int zoom);

unsigned char getUart();

void wait_msec(unsigned int msVal);

void set_wait_timer(int set, unsigned int msVal);

int strComp(char *str1, char *str2);

int startsWith(char *full, char *prefix);

int is_all_digits(const char *str);

int my_atoi(const char *str);

void strCopy(char *dest, const char *src);

int strLen(const char *str);

void intToStr(int num, char *str);

