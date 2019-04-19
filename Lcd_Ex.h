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
#define CONFIG_LCD_TYPE_ILI9341

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

void lcd_cmd(spi_device_handle_t spi_device_handle, const uint8_t cmd) {
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi_device_handle, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

void lcd_data(spi_device_handle_t spi_device_handle, const uint8_t *data, int len) {
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi_device_handle, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void lcd_init(spi_device_handle_t spi_device_handle) {
    int cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    lcd_init_cmds = ili_init_cmds;
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(spi_device_handle, lcd_init_cmds[cmd].cmd);
        lcd_data(spi_device_handle, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        if (lcd_init_cmds[cmd].databytes&0x80) vTaskDelay(100 / portTICK_RATE_MS);
        cmd++;
    }
    gpio_set_level(PIN_NUM_BCKL, 1); ///Enable backlight
}


static void send_lines(spi_device_handle_t spi_device_handle, int ypos, uint16_t *linedata) {
    esp_err_t ret;
    int x;
    static spi_transaction_t trans[6];
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;           //Column Address Set
    trans[1].tx_data[0]=0;              //Start Col High
    trans[1].tx_data[1]=0;              //Start Col Low
    trans[1].tx_data[2]=(320)>>8;       //End Col High
    trans[1].tx_data[3]=(320)&0xff;     //End Col Low
    trans[2].tx_data[0]=0x2B;           //Page address set
    trans[3].tx_data[0]=ypos>>8;        //Start page high
    trans[3].tx_data[1]=ypos&0xff;      //start page low
    trans[3].tx_data[2]=(ypos+PARALLEL_LINES)>>8;    //end page high
    trans[3].tx_data[3]=(ypos+PARALLEL_LINES)&0xff;  //end page low
    trans[4].tx_data[0]=0x2C;           //memory write
    trans[5].tx_buffer=linedata;        //finally send the line data
    trans[5].length=320*2*8*PARALLEL_LINES;          //Data length, in bits
    trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag
    for (x=0; x<6; x++) {
        ret=spi_device_queue_trans(spi_device_handle, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }
}

static void send_line_finish(spi_device_handle_t spi_device_handle)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    for (int x=0; x<6; x++) {
        ret=spi_device_get_trans_result(spi_device_handle, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
    }
}

spi_device_handle_t Init() {
    esp_err_t ret;
    spi_device_handle_t spi_device_handle;
    spi_bus_config_t buscfg={
        .mosi_io_num=PIN_NUM_MOSI,
        .miso_io_num=PIN_NUM_MISO,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=PARALLEL_LINES*320*2+8,
        .flags = 0,
        .intr_flags = 0
    };
    spi_device_interface_config_t devcfg={
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode=0,                                //SPI mode 0
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz=10*1000*1000,           //Clock out at 10 MHz
        .input_delay_ns = 0,
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .flags = 0,
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
        .post_cb = 0
    };
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi_device_handle);
    ESP_ERROR_CHECK(ret);
    lcd_init(spi_device_handle);
    return spi_device_handle;
}