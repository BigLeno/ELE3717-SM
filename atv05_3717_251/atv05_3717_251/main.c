#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// --- Definições LCD ---
#define LCD_PORT PORTD
#define LCD_DDR DDRD
#define RS PD2
#define E  PD3

// --- Funções SPI e MAX7219 ---
#define DATA_PORT_PORT PORTB
#define DATA_PORT_DDR DDRB
#define DIN PB3     // SPI MOSI
#define CS  PB2     // Chip select
#define CLK PB5     // SPI Clock

void spi_init() {
    DATA_PORT_DDR |= (1 << DIN) | (1 << CS) | (1 << CLK);
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    DATA_PORT_PORT |= (1 << CS);
}

void spi_transmit(uint8_t data) {
    SPDR = data;
    while(!(SPSR & (1 << SPIF)));
}

void max7219_send(uint8_t addr, uint8_t data) {
    DATA_PORT_PORT &= ~(1 << CS);
    spi_transmit(addr);
    spi_transmit(data);
    DATA_PORT_PORT |= (1 << CS);
}

void max7219_init() {
    max7219_send(0x0F, 0x00);
    max7219_send(0x09, 0x00);
    max7219_send(0x0A, 0x03);
    max7219_send(0x0B, 0x07);
    max7219_send(0x0C, 0x01);
}

void max7219_clear() {
    for (uint8_t i = 1; i <= 8; i++) {
        max7219_send(i, 0x00);
    }
}

// --- Funções ADC ---

void adc_init() {
    ADMUX = (1 << REFS0); // AVcc ref
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    DIDR0 = (1 << ADC4D) | (1 << ADC5D);
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADCW;
}

// --- Funções LCD ---

void lcd_pulse_enable() {
    LCD_PORT |= (1 << E);
    _delay_us(1);
    LCD_PORT &= ~(1 << E);
    _delay_us(100);
}

void lcd_send_nibble(uint8_t nibble) {
    LCD_PORT &= 0x0F;
    LCD_PORT |= (nibble << 4);
    lcd_pulse_enable();
}

void lcd_send_byte(uint8_t data, uint8_t is_data) {
    if (is_data)
        LCD_PORT |= (1 << RS);
    else
        LCD_PORT &= ~(1 << RS);
    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
    _delay_us(50);
}

void lcd_init() {
    LCD_DDR |= (1 << RS) | (1 << E) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
    _delay_ms(40);
    LCD_PORT &= ~((1 << RS) | (1 << E) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));

    lcd_send_nibble(0x03);
    _delay_ms(5);
    lcd_send_nibble(0x03);
    _delay_us(150);
    lcd_send_nibble(0x03);
    lcd_send_nibble(0x02);

    lcd_send_byte(0x28, 0);
    lcd_send_byte(0x0C, 0);
    lcd_send_byte(0x06, 0);
    lcd_send_byte(0x01, 0);
    _delay_ms(2);
}

void lcd_clear() {
    lcd_send_byte(0x01, 0);
    _delay_ms(2);
}

void lcd_goto(uint8_t line, uint8_t pos) {
    uint8_t addr = pos + (line ? 0x40 : 0x00);
    lcd_send_byte(0x80 | addr, 0);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send_byte(*str++, 1);
    }
}

// --- Função para mostrar bits como string com espaços ---
void bits_to_string(uint8_t val, char* str) {
    for (uint8_t i = 0; i < 8; i++) {
        str[i * 2] = (val & (1 << (7 - i))) ? '1' : '0';
        str[i * 2 + 1] = ' ';
    }
    str[16] = '\0';
}

// --- Main ---

int main() {
    spi_init();
    max7219_init();
    max7219_clear();
    adc_init();
    lcd_init();

    int pos_x = 0, pos_y = 0;
    int last_pos_x = -1, last_pos_y = -1;
    char line_bits_str[17];
    char col_bits_str[17];

    while (1) {
        uint16_t adc_x = adc_read(4);
        uint16_t adc_y = adc_read(5);

        pos_x = (adc_x * 8) / 1024;
        pos_y = (adc_y * 8) / 1024;
        if (pos_x > 7) pos_x = 7;
        if (pos_y > 7) pos_y = 7;

        if (pos_x != last_pos_x || pos_y != last_pos_y) {
            if (last_pos_y >= 0 && last_pos_y <= 7) {
                max7219_send(last_pos_y + 1, 0x00);
            }

            uint8_t line_val = 1 << pos_x;
            max7219_send(pos_y + 1, line_val);

            uint8_t col_val = 1 << pos_y;

            bits_to_string(line_val, line_bits_str);
            bits_to_string(col_val, col_bits_str);

            lcd_clear();
            lcd_goto(0, 0);
            lcd_print("X:");
            lcd_print(line_bits_str);
            lcd_goto(1, 0);
            lcd_print("Y:");
            lcd_print(col_bits_str);

            last_pos_x = pos_x;
            last_pos_y = pos_y;
        }

        _delay_ms(100);
    }
}