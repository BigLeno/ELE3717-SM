#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

#define set_bit(Y, bit_x)  ((Y) |= (1 << (bit_x)))
#define clr_bit(Y, bit_x)  ((Y) &= ~(1 << (bit_x)))

// LCD 4 bits: dados nos PD4..PD7; RS=PD2, EN=PD3
#define PORT_LCD PORTD
#define DDR_LCD  DDRD

#define RS PD2
#define EN PD3

#define LCD_DATA_MASK ((1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7))

#define BTN1_PIN PC1  // Botão para trocar canal (S1)
#define BTN2_PIN PC2  // Botão para aumentar (S2)
#define BTN3_PIN PC3  // Botão para diminuir (S3)

// Protótipos
void lcd_init(void);
void lcd_send_nibble(uint8_t val);
void lcd_send_byte(uint8_t c, uint8_t rs);
void lcd_clear(void);
void lcd_goto(uint8_t linha, uint8_t coluna);
void lcd_print(char *str);
void lcd_print_num(uint8_t val);
bool button_pressed(uint8_t pin);

// Variáveis de controle
volatile uint8_t red_val = 0;
volatile uint8_t green_val = 0;
volatile uint8_t blue_val = 0;
volatile uint8_t seletor = 0;  // 0=RED,1=GREEN,2=BLUE

// Funções LCD
void lcd_send_nibble(uint8_t val)
{
    PORT_LCD &= ~LCD_DATA_MASK;       // Limpa bits D4-D7
    PORT_LCD |= ((val << 4) & LCD_DATA_MASK);  // Posiciona nibble
    set_bit(PORT_LCD, EN);
    _delay_us(1);
    clr_bit(PORT_LCD, EN);
    _delay_us(40);
}

void lcd_send_byte(uint8_t c, uint8_t rs)
{
    if (rs)
        set_bit(PORT_LCD, RS);
    else
        clr_bit(PORT_LCD, RS);
    lcd_send_nibble(c >> 4);
    lcd_send_nibble(c & 0x0F);
    if (rs == 0 && (c == 0x01 || c == 0x02)) // clear cursor commands
        _delay_ms(2);
    else
        _delay_us(40);
}

void lcd_init(void)
{
    DDR_LCD |= LCD_DATA_MASK | (1 << RS) | (1 << EN);
    _delay_ms(40);
    clr_bit(PORT_LCD, RS);

    // Sequencia inicialização 4bits
    lcd_send_nibble(0x03);
    _delay_ms(5);
    lcd_send_nibble(0x03);
    _delay_us(150);
    lcd_send_nibble(0x03);
    _delay_us(150);
    lcd_send_nibble(0x02); // set 4-bit
    _delay_us(150);
    lcd_send_byte(0x28, 0); // 4-bit, 2 linhas, 5x8 fonte
    lcd_send_byte(0x08, 0); // Display off
    lcd_send_byte(0x01, 0); // clear display
    _delay_ms(2);
    lcd_send_byte(0x06, 0); // entrada incrementa, sem shift
    lcd_send_byte(0x0C, 0); // Display ON, Cursor off, blink off
}

void lcd_clear(void)
{
    lcd_send_byte(0x01, 0);
    _delay_ms(2);
}

void lcd_goto(uint8_t linha, uint8_t coluna)
{
    uint8_t endereco = coluna + (linha ? 0x40 : 0x00);
    lcd_send_byte(0x80 | endereco, 0);
}

void lcd_print(char *str)
{
    while (*str)
        lcd_send_byte(*str++, 1);
}

void lcd_print_num(uint8_t val)
{
    char buffer[4];
    // imprime valor decimal com três dígitos
    buffer[0] = (val / 100) + '0';
    buffer[1] = ((val / 10) % 10) + '0';
    buffer[2] = (val % 10) + '0';
    buffer[3] = 0;
    lcd_print(buffer);
}

// Setup dos botões com pull-up
void buttons_init(void)
{
    DDRC &= ~((1 << BTN1_PIN) | (1 << BTN2_PIN) | (1 << BTN3_PIN)); // entrada
    PORTC |= (1 << BTN1_PIN) | (1 << BTN2_PIN) | (1 << BTN3_PIN);    // pull-ups
}

// Leitura simples dos botões, retorna true se pressionado (ativo-baixo)
bool button_pressed(uint8_t pin)
{
    if (!(PINC & (1 << pin)))
    {
        _delay_ms(20); // debounce básico
        if (!(PINC & (1 << pin)))
            return true;
    }
    return false;
}

void lcd_atualiza_display(void)
{
    lcd_clear();

    // top row - títulos
    lcd_goto(0, 0);
    lcd_print("RED  GREEN BLUE");

    // segunda linha valores e indicação da cor selecionada
    lcd_goto(1, 0);

    // imprime RED
    if (seletor == 0)
        lcd_send_byte('_', 1);  // underline indica seleção
    else
        lcd_send_byte(' ', 1);
    lcd_print_num(red_val);
    lcd_print(" ");

    // imprime GREEN
    if (seletor == 1)
        lcd_send_byte('_', 1);
    else
        lcd_send_byte(' ', 1);
    lcd_print_num(green_val);
    lcd_print(" ");

    // imprime BLUE
    if (seletor == 2)
        lcd_send_byte('_', 1);
    else
        lcd_send_byte(' ', 1);
    lcd_print_num(blue_val);
}

int main(void)
{
    buttons_init();
    lcd_init();
    lcd_atualiza_display();

    for (;;)
    {
        // Troca cor selecionada com o botão S1
        if (button_pressed(BTN1_PIN))
        {
            seletor++;
            if (seletor > 2)
                seletor = 0;
            lcd_atualiza_display();
            while (button_pressed(BTN1_PIN))
                ; // espera liberar botão
        }

        // Incrementa valor com S2
        if (button_pressed(BTN2_PIN))
        {
            switch (seletor)
            {
            case 0:
                if (red_val < 255)
                    red_val++;
                break;
            case 1:
                if (green_val < 255)
                    green_val++;
                break;
            case 2:
                if (blue_val < 255)
                    blue_val++;
                break;
            }
            lcd_atualiza_display();
            while (button_pressed(BTN2_PIN))
                ; // espera liberar botão
        }

        // Decrementa valor com S3
        if (button_pressed(BTN3_PIN))
        {
            switch (seletor)
            {
            case 0:
                if (red_val > 0)
                    red_val--;
                break;
            case 1:
                if (green_val > 0)
                    green_val--;
                break;
            case 2:
                if (blue_val > 0)
                    blue_val--;
                break;
            }
            lcd_atualiza_display();
            while (button_pressed(BTN3_PIN))
                ; // espera liberar botão
        }
    }
}