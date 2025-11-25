#include "i2c-lcd.h"
// Tiempo seguros (ms). Puedes reducir una vez validado (pero no demasiado).
#define T_WRITE_STABILIZE   2    // tiempo tras HAL_I2C_Master_Transmit para que PCF piense
#define T_EN_HIGH_MS        3    // tiempo en EN=1 (mantener para captura segura)
#define T_EN_LOW_MS         2    // tiempo entre bajar EN y siguiente acción
#define T_INIT_LONG_MS     50
#define T_INIT_STEP_MS      5

static uint8_t backlight = LCD_BACKLIGHT; // cambiar a 0 si querés por defecto apagado

// Escribe byte crudo al PCF8574 (bloqueante)
static HAL_StatusTypeDef pcf_write_raw(uint8_t data)
{
    HAL_StatusTypeDef st = HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, &data, 1, 200);
    // pequeña espera para que las salidas se estabilicen
    HAL_Delay(T_WRITE_STABILIZE);
    return st;
}

// Pulso EN: envía DATA con EN=1 (una transferencia) y luego EN=0 (otra transferencia)
static void lcd_pulse(uint8_t data)
{
    // Asegurar que data ya contiene backlight, RS/RW y nibble en P4..P7
    pcf_write_raw(data | LCD_EN);   // EN = 1
    HAL_Delay(T_EN_HIGH_MS);
    pcf_write_raw(data & ~LCD_EN);  // EN = 0
    HAL_Delay(T_EN_LOW_MS);
}

// Envía un nibble (ya en posición alta, p.ej. (value & 0xF0)) con RS según flag
static void lcd_write_nibble(uint8_t nibble_high_with_ctrl)
{
    // nibble_high_with_ctrl debe incluir backlight y RS/RW si corresponde.
    // Ej: (value & 0xF0) | backlight | (rs ? LCD_RS : 0)
    lcd_pulse(nibble_high_with_ctrl);
}

// Envía byte (comando o dato). rs = 0 comando, rs = 1 dato
static void lcd_send_byte(uint8_t value, uint8_t rs)
{
    uint8_t high = (value & 0xF0) | backlight | (rs ? LCD_RS : 0);
    uint8_t low  = ((value << 4) & 0xF0) | backlight | (rs ? LCD_RS : 0);

    // IMPORTANTE: hacemos cada nibble con su propio pcf_write + pulse
    lcd_write_nibble(high);
    lcd_write_nibble(low);

    // pequeño delay para que el HD44780 procese el byte
    HAL_Delay(2);
}

// API pública
void lcd_command(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);
}

void lcd_putc(char c)
{
    lcd_send_byte((uint8_t)c, 1);
}

void lcd_puts(const char *str)
{
    while (*str) {
        lcd_putc(*str++);
    }
}

void lcd_clear(void)
{
    lcd_command(0x01);
    HAL_Delay(2); // clear necesita más tiempo
}

void lcd_home(void)
{
    lcd_command(0x02);
    HAL_Delay(2);
}

// cursor row,col (row: 0 o 1, col: 0..15)
void lcd_goto(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    addr += col;
    lcd_command(0x80 | addr);
}

// Inicialización conforme datasheet, usando pulses robustos
void lcd_init(void)
{
    HAL_Delay(T_INIT_LONG_MS); // >40ms power-up

    // Asegurar líneas a estado inicial
    pcf_write_raw(backlight & LCD_BACKLIGHT); // RS=0,RW=0,EN=0,BL según backlight

    // Mandar 0x30 tres veces (upper nibble) con delays
    // En modo 8-bit esto fuerza el init; aquí mandamos solo el nibble alto a través del PCF
    lcd_pulse(0x30 | backlight); HAL_Delay(T_INIT_STEP_MS);
    lcd_pulse(0x30 | backlight); HAL_Delay(T_INIT_STEP_MS);
    lcd_pulse(0x30 | backlight); HAL_Delay(T_INIT_STEP_MS);

    // Ahora forzamos 4-bit mode con 0x20 (nibble alto = 0x20)
    lcd_pulse(0x20 | backlight); HAL_Delay(T_INIT_STEP_MS);

    // Ya en modo 4 bits, enviar configuración completa (cada cmd usa 2 nibbles)
    lcd_command(0x28); // function set: 4-bit, 2 lines, 5x8 dots
    lcd_command(0x08); // display off
    lcd_clear();       // clear display
    lcd_command(0x06); // entry mode set: increment
    lcd_command(0x0C); // display on, cursor off
}
