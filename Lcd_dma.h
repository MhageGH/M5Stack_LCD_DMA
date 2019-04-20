#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK GPIO_NUM_18
#define PIN_NUM_CS GPIO_NUM_14
#define PIN_NUM_DC GPIO_NUM_27
#define PIN_NUM_RST GPIO_NUM_33
#define PIN_NUM_BCKL GPIO_NUM_32

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

    void CreateFramebuffer();
    DRAM_ATTR static const lcd_init_cmd_t ili_init_cmds[];
    void lcd_cmd(spi_device_handle_t hSpi, const uint8_t cmd);
    void lcd_data(spi_device_handle_t hSpi, const uint8_t *data, int len);
    static void lcd_spi_pre_transfer_callback(spi_transaction_t *t); //This function is called (in irq context!) just before a transmission starts. It will set the D/C line to the value indicated in the user field.
    void lcd_init(spi_device_handle_t hSpi);
    void send_framebuffer(spi_device_handle_t hSpi, int x, int y, int w, int h, uint16_t *linedata);
    void send_framebuffer_finish(spi_device_handle_t hSpi);

public:
    Lcd_dma(int width, int height);
    ~Lcd_dma();
    void Flip(int x, int y);
    uint16_t *GetFramebuffer();
    int GetWidth();
    int GetHeight();
    void fillScreen(uint16_t color);
};