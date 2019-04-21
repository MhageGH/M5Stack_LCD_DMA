# M5Stack_LCD_DMA
M5Stack library (https://github.com/m5stack/M5Stack) has great display class.
It uses polling SPI transactions to transmit data to LCD.
But when you need to calculate efficiently, you want to use interrupt SPI transactions with DMA.
This is simple class which allows you to use it.