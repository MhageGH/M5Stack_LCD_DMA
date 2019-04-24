#include "Lcd_dma.h"

DRAM_ATTR const Lcd_dma::lcd_init_cmd_t Lcd_dma::ili_init_cmds[] = {
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

    {0x36, {0x08}, 1}, // ref to setRotation(1) in ILI9341_Rotation.h in M5Stack Library
    {0, {0}, 0xff}     // end
};

spi_device_handle_t Lcd_dma::spi_start()
{
    esp_err_t ret;
    spi_device_handle_t hSpi;
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = height * width * 2 + 8,
        .flags = 0,
        .intr_flags = 0};
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0, //SPI mode 0
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = 40 * 1000 * 1000,
        .input_delay_ns = 0,
        .spics_io_num = PIN_NUM_CS, //CS pin
        .flags = 0,
        .queue_size = 7,                         //We want to be able to queue 7 transactions at a time
        .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .post_cb = 0};
    ret = spi_bus_initialize(spi_host, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    ret = spi_bus_add_device(spi_host, &devcfg, &hSpi);
    ESP_ERROR_CHECK(ret);
    return hSpi;
}

void Lcd_dma::CreateFramebuffer()
{
    for (int i = 0; i < 2; i++)
    {
        framebuffers[i] = (uint16_t *)heap_caps_malloc(width * height * sizeof(uint16_t), MALLOC_CAP_DMA);
        assert(framebuffers[i] != NULL);
    }
}

void Lcd_dma::lcd_cmd(spi_device_handle_t hSpi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));                    //Zero out the transaction
    t.length = 8;                                //Command is 8 bits
    t.tx_buffer = &cmd;                          //The data is the cmd itself
    t.user = (void *)0;                          //D/C needs to be set to 0
    ret = spi_device_polling_transmit(hSpi, &t); //Transmit!
    assert(ret == ESP_OK);                       //Should have had no issues.
}

void Lcd_dma::lcd_data(spi_device_handle_t hSpi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0)
        return;                                  //no need to send anything
    memset(&t, 0, sizeof(t));                    //Zero out the transaction
    t.length = len * 8;                          //Len is in bytes, transaction length is in bits.
    t.tx_buffer = data;                          //Data
    t.user = (void *)1;                          //D/C needs to be set to 1
    ret = spi_device_polling_transmit(hSpi, &t); //Transmit!
    assert(ret == ESP_OK);                       //Should have had no issues.
}

void Lcd_dma::lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void Lcd_dma::lcd_init(spi_device_handle_t hSpi)
{
    int cmd = 0;
    const lcd_init_cmd_t *lcd_init_cmds;
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    lcd_init_cmds = ili_init_cmds;
    while (lcd_init_cmds[cmd].databytes != 0xff)
    {
        lcd_cmd(hSpi, lcd_init_cmds[cmd].cmd);
        lcd_data(hSpi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
        if (lcd_init_cmds[cmd].databytes & 0x80)
            vTaskDelay(100 / portTICK_RATE_MS);
        cmd++;
    }
    gpio_set_level(PIN_NUM_BCKL, 1); ///Enable backlight
}

void Lcd_dma::send_framebuffer(spi_device_handle_t hSpi, int x, int y, int w, int h, uint16_t *framebuffer)
{
    esp_err_t ret;
    static spi_transaction_t trans[6];
    for (int i = 0; i < 6; i++)
    {
        memset(&trans[i], 0, sizeof(spi_transaction_t));
        if ((i & 1) == 0)
        {
            trans[i].length = 8;
            trans[i].user = (void *)0;
        }
        else
        {
            trans[i].length = 8 * 4;
            trans[i].user = (void *)1;
        }
        trans[i].flags = SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0] = 0x2A;               //Column Address Set
    trans[1].tx_data[0] = x >> 8;             //Start Col High
    trans[1].tx_data[1] = x & 0xFF;           //Start Col Low
    trans[1].tx_data[2] = (x + w - 1) >> 8;   //End Col High
    trans[1].tx_data[3] = (x + w - 1) & 0xFF; //End Col Low
    trans[2].tx_data[0] = 0x2B;               //Page address set
    trans[3].tx_data[0] = y >> 8;             //Start page high
    trans[3].tx_data[1] = y & 0xFF;           //start page low
    trans[3].tx_data[2] = (y + h - 1) >> 8;   //end page high
    trans[3].tx_data[3] = (y + h - 1) & 0xFF; //end page low
    trans[4].tx_data[0] = 0x2C;               //memory write
    trans[5].tx_buffer = framebuffer;         //finally send the line data
    trans[5].length = w * 2 * 8 * h;          //Data length, in bits
    trans[5].flags = 0;                       //undo SPI_TRANS_USE_TXDATA flag
    for (int i = 0; i < 6; i++)
    {
        ret = spi_device_queue_trans(hSpi, &trans[i], portMAX_DELAY);
        assert(ret == ESP_OK);
    }
}

void Lcd_dma::send_framebuffer_finish(spi_device_handle_t hSpi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    for (int i = 0; i < 6; ++i)
    {
        ret = spi_device_get_trans_result(hSpi, &rtrans, portMAX_DELAY);
        assert(ret == ESP_OK);
    }
}

Lcd_dma::Lcd_dma(int width, int height)
{
    this->width = width;
    this->height = height;
    hSpi_m = spi_start();
    lcd_init(hSpi_m);
    CreateFramebuffer();
}

Lcd_dma::~Lcd_dma()
{
    if (sending_framebuffer != -1)
        send_framebuffer_finish(hSpi_m);
    spi_bus_remove_device(hSpi_m);
    spi_bus_free(spi_host);
    for (int i = 0; i < 2; ++i)
        free(framebuffers[i]);
}

void Lcd_dma::Flip(int x, int y)
{
    if (sending_framebuffer != -1)
        send_framebuffer_finish(hSpi_m);
    sending_framebuffer = calc_framebuffer;
    calc_framebuffer = (calc_framebuffer == 1) ? 0 : 1;
    send_framebuffer(hSpi_m, x, y, width, height, framebuffers[sending_framebuffer]);
}

uint16_t *Lcd_dma::GetFramebuffer()
{
    return framebuffers[calc_framebuffer];
}

int Lcd_dma::GetHeight()
{
    return height;
}

int Lcd_dma::GetWidth()
{
    return width;
}

void Lcd_dma::fillScreen(uint16_t color)
{
    uint16_t *buf = (uint16_t *)heap_caps_malloc(320 * 1 * sizeof(uint16_t), MALLOC_CAP_DMA);
    for (int i = 0; i < 320; ++i)
        buf[i] = color;
    for (int i = 0; i < 240; ++i)
    {
        send_framebuffer(hSpi_m, 0, i, 320, 1, buf);
        send_framebuffer_finish(hSpi_m);
    }
    free(buf);
    sending_framebuffer = -1;
}

void Lcd_dma::SpiFree()
{
    if (sending_framebuffer != -1)
        send_framebuffer_finish(hSpi_m);
    spi_bus_remove_device(hSpi_m);
    spi_bus_free(spi_host);
    sending_framebuffer = -1;
}

void Lcd_dma::SpiRestart()
{
    hSpi_m = spi_start();
}

void Lcd_dma::SetBrightness(int brightness)
{
    // for M5Stack backlight PWM control.
    ledcSetup(7, 44100, 8);
    ledcAttachPin(PIN_NUM_BCKL, 7);
    ledcWrite(7, brightness);
}