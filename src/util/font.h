#ifndef CUSTOM_FONT_H
#define CUSTOM_FONT_H

enum {
    FONT_WIDTH = 8,
    FONT_HEIGHT = 15,
    FONT_BPL = 2,
    FONT_BPG = 30,
    FONT_NUMGLYPHS = 256
};

unsigned short font[256][15];
#endif // CUSTOM_FONT_H