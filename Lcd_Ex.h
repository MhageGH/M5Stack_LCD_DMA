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
#define PARALLEL_LINES 120

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

typedef enum {
    LCD_TYPE_ILI = 1,
    LCD_TYPE_ST,
    LCD_TYPE_MAX,
} type_lcd_t;

DRAM_ATTR static const lcd_init_cmd_t ili_init_cmds[] = {
    // ref to TFT_eSPI::init() In_eSPI.cpp in M5Stack Libarary
    {0xEF, {0x03, 0x80, 0x02}, 3},
    {0xCF, {0x00, 0xC1, 0x30}, 3},
    {0xED, {0x64, 0x03, 0x12, 0x81}, 4},
    {0xE8, {0x85, 0x00, 0x78}, 3},
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {0xC0, {0x23}, 1},
    {0xC1, {0x10}, 1},
    {0xC5, {0x3e, 0x28}, 2},
    {0xC7, {0x86}, 1},
    {0x36, {0xA8}, 1},
    {0x3A, {0x55}, 1},
    {0xB1, {0x00, 0x13}, 2},
    {0xB6, {0x08, 0x82, 0x27}, 3},
    {0xF2, {0x00}, 1},
    {0x26, {0x01}, 1},
    {0xE0, {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00}, 15},
    {0xE1, {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F}, 15},
    {0x11, {0}, 0x80},
    {0x29, {0}, 0x80},
    
    {0x36, {0x08}, 1},  // ref to setRotation(1) in ILI9341_Rotation.h in M5Stack Library
    {0, {0}, 0xff}      // end
};

void lcd_cmd(spi_device_handle_t spi_device_handle, const uint8_t cmd);

void lcd_data(spi_device_handle_t spi_device_handle, const uint8_t *data, int len);

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t);

void lcd_init(spi_device_handle_t spi_device_handle);

void send_lines(spi_device_handle_t spi_device_handle, int ypos, uint16_t *linedata);

void send_line_finish(spi_device_handle_t spi_device_handle);

spi_device_handle_t Init();