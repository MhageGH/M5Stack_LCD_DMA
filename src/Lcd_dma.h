#pragma once

#include "driver/spi_master.h"

class Lcd_dma
{
    typedef struct
    {
        uint8_t cmd;
        uint8_t data[16];
        uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
    } lcd_init_cmd_t;
    typedef enum
    {
        LCD_TYPE_ILI = 1,
        LCD_TYPE_ST,
        LCD_TYPE_MAX,
    } type_lcd_t;

    uint16_t *framebuffers[2];
    int width, height;
    int sending_framebuffer = -1;
    int calc_framebuffer = 0;
    spi_device_handle_t hSpi_m;
    spi_host_device_t spi_host = VSPI_HOST;

    spi_device_handle_t spi_start();
    void CreateFramebuffer();
    DRAM_ATTR static const lcd_init_cmd_t ili_init_cmds[];
    void lcd_cmd(spi_device_handle_t hSpi, const uint8_t cmd);
    void lcd_data(spi_device_handle_t hSpi, const uint8_t *data, int len);
    static void lcd_spi_pre_transfer_callback(spi_transaction_t *t); //This function is called (in irq context!) just before a transmission starts. It will set the D/C line to the value indicated in the user field.
    void lcd_init(spi_device_handle_t hSpi);
    void send_framebuffer(spi_device_handle_t hSpi, int x, int y, int w, int h, uint16_t *linedata);
    void send_framebuffer_finish(spi_device_handle_t hSpi);

public:
    // @param width and height of drawing area
    Lcd_dma(int width, int height);
    ~Lcd_dma();
    int GetWidth();
    int GetHeight();
    // @param brightness 0~255
    void SetBrightness(int brightness);
    void SetSize(int width, int height);

    // You can get framebuffer (back buffer) with this function and revise it.
    // 16bit RGB (5-6-5) colors should be expressed with little endian.
    uint16_t *GetFramebuffer();

    // 1. Wait the end of previous transmit.
    // 2. Flip the front buffer and back buffer. Back buffer has been revised by you.
    // 3. Start next transmit with front buffer.
    // @param position of drawing area
    void Flip(int x, int y);

    // @param color : 16bit RGB (5-6-5) color should be expressed with little endian.
    void fillScreen(uint16_t color);

    void SpiFree();
    void SpiRestart();
};