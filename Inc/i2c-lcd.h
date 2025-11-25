#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_

#include "main.h"
#include "stm32f4xx_hal.h"

// Ajustá la dirección si tu módulo usa otra (0x27 ó 0x3F comúnmente)
#ifndef SLAVE_ADDRESS_LCD
#define SLAVE_ADDRESS_LCD  (0x27 << 1)
#endif

extern I2C_HandleTypeDef hi2c1; // cambia si usás otro handler

// Bits según mapeo confirmado
#define LCD_RS       (1 << 0)  // P0
#define LCD_RW       (1 << 1)  // P1
#define LCD_EN       (1 << 2)  // P2
#define LCD_BACKLIGHT (1 << 3) // P3

// D4..D7 = P4..P7 (se envían en los nibble altos de un byte)
#define DATA_MASK 0xF0

// API
void lcd_init(void);
void lcd_clear(void);
void lcd_home(void);
void lcd_putc(char c);
void lcd_puts(const char *str);
void lcd_goto(uint8_t row, uint8_t col);
void lcd_command(uint8_t cmd);

#endif /* INC_I2C_LCD_H_ */
