#include "Lcd_dma.h"

void SetBuf(uint16_t* buf, int y) {
    for (int i = 0; i < PARALLEL_LINES; ++i) {
        for (int j = 0; j < 320; ++j) {
            uint16_t r = (uint16_t)(31.0f * (float)(j) / 320.0f);
            uint16_t g = (uint16_t)(63.0f * (float)(j) / 320.0f);
            uint16_t b = (uint16_t)(31.0f * (float)(j) / 320.0f);
            uint16_t c = (r << 11) | (g << 5) | (b << 0);
            c = (c << 8) | (c >> 8);
            buf[320 * i + j] = c;
        }
    }
}

void setup() {
    Init();
}

void loop() {
    static uint16_t* buf = GetBuf();
    for (int y = 0; y < 240; y += PARALLEL_LINES) {
        SetBuf(buf, y);
        delay(60);
        Revise(y);
        buf = GetBuf();
    }
}