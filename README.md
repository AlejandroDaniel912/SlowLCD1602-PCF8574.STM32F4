# SlowLCD1602-PCF8574.STM32F4
A useful driver for the LCD11602, which is not capable of reading fast changes on the ENABLE signal. This driver executes two instruction cycles per character instead of one. If your LCD only allows reading the upper 4 bits while the lower 4 bits are always 1111, this implementation may be helpful.
