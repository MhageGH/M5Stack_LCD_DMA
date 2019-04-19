#include "SPI_Master.h"

uint16_t *lines[2];
spi_device_handle_t spi_device_handle;
int sending_line=-1;
int calc_line=0;

void CreateBuffer() {
    for (int i=0; i<2; i++) {
        lines[i] = (uint16_t*)heap_caps_malloc(320 * PARALLEL_LINES * sizeof(uint16_t), MALLOC_CAP_DMA);
        assert(lines[i]!=NULL);
    }
}

void calc_lines(uint16_t* line, int y) {
    for (int i = 0; i < PARALLEL_LINES; ++i) {
        for (int j = 0; j < 320; ++j) {
            uint16_t r = (uint16_t)(31.0f * (float)(j) / 320.0f);
            uint16_t g = (uint16_t)(63.0f * (float)(j) / 320.0f);
            uint16_t b = (uint16_t)(31.0f * (float)(j) / 320.0f);
            uint16_t c = (r << 11) | (g << 5) | (b << 0);
            c = (c << 8) | (c >> 8);
            line[320 * i + j] = c;
        }
    }
}

void setup() {
    spi_device_handle = Init();
    CreateBuffer();
}

void loop() {
    for (int y = 0; y < 240; y += PARALLEL_LINES) {
        calc_lines(lines[calc_line], y);
        delay(60);
        if (sending_line != -1) send_line_finish(spi_device_handle);
        sending_line = calc_line;
        calc_line = (calc_line == 1) ? 0 : 1;
        send_lines(spi_device_handle, y, lines[sending_line]);
    }
}