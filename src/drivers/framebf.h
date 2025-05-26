// ----------------------------------- framebf.h -------------------------------------
void framebf_init();

void drawPixelARGB32(int x, int y, unsigned int attr);

void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);

void drawLine(int x1, int y1, int x2, int y2, unsigned int attr);

void drawLCircle(int center_x, int center_y, int radius, unsigned int attr, int fill);

void drawBackground(unsigned int color);

void drawImg(const unsigned long pixel_data[], int pos_y, int pos_x, int pic_width, int pic_height, int isTrans);
void drawImg2(const unsigned long pixel_data[], int pos_y, int pos_x, int pic_width, int pic_height);

void clearScreen();