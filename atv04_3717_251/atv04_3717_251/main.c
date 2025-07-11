/*
 * atv04_3717_251.c
 *
 * Created: 30/06/2025 19:22:57
 * Author : livia
 */ 

/*
 * Projeto 04.c
 *
 * Created: 30/05/2025 15:38:20
 * Author : livia
 */ 

#define F_CPU 16000000UL

// Bibliotecas
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdint.h>

// Defini??es para LCD 4 bits em PORTD
#define PORT_LCD PORTD
#define DDR_LCD  DDRD
#define RS PD2
#define EN PD3
#define LCD_DATA_MASK ((1 << PD4)|(1 << PD5)|(1 << PD6)|(1 << PD7))


// Bot?es nos pinos PC1, PC2, PC3
#define BTN_S1 PC1
#define BTN_S2 PC2
#define BTN_S3 PC3

#define set_bit(Y, bit_x)  ((Y) |= (1 << (bit_x)))
#define clr_bit(Y, bit_x)  ((Y) &= ~(1 << (bit_x)))


// Vari?veis controle
volatile uint8_t red_val = 0;
volatile uint8_t green_val = 0;
volatile uint8_t blue_val = 0;
volatile int8_t seletor = -1; // -1 = desativado, 0=RED,1=GREEN,2=BLUE

volatile uint8_t incremento = 0;
volatile uint8_t decremento = 0;

volatile int8_t passo = 1;
volatile int16_t contagem = 0;

// Para controle de atualiza??o parcial
uint8_t old_red = 255;
uint8_t old_green = 255;
uint8_t old_blue = 255;
int8_t old_seletor = -2;

// Colunas base para os valores na linha 1
const uint8_t col_base[3] = {0, 7, 12};

// Fun??es LCD:

//Envia pulso no pino Enable
#define pulso_enable()       \
{                       \
	set_bit(PORT_LCD, EN); \
	_delay_us(1);        \
	clr_bit(PORT_LCD, EN); \
	_delay_us(100);      \
}


void lcd_enviar_nibble(uint8_t val) {
	// limpa as linhas D4-D7
	PORT_LCD &= ~LCD_DATA_MASK;
	// posiciona os 4 bits em D4..D7
	PORT_LCD |= (val << 4) & LCD_DATA_MASK;
	pulso_enable();
}

// Envia um byte inteiro dividindo em dois nibbles para modo 4 bits
void cmd_LCD(uint8_t c, uint8_t rs) {
	if (rs)
	set_bit(PORT_LCD, RS);
	else
	clr_bit(PORT_LCD, RS);
	lcd_enviar_nibble(c >> 4);
	lcd_enviar_nibble(c & 0x0F);
	if (rs == 0 && c < 4)
	_delay_ms(2);
	else
	_delay_us(40);
}

// Inicializa o LCD para modo 4 bits, 2 linhas, 5x8 dots
void inic_LCD_4bits(void) {
	DDR_LCD |= LCD_DATA_MASK | (1 << RS) | (1 << EN);
	_delay_ms(40); // Delay power on
	// Inicialização especial modo 4 bits (sequência do datasheet)
	clr_bit(PORT_LCD, RS);
	lcd_enviar_nibble(0x03);
	_delay_ms(5);
	lcd_enviar_nibble(0x03);
	_delay_us(150);
	lcd_enviar_nibble(0x03);
	_delay_us(150);
	lcd_enviar_nibble(0x02); // Modo 4 bits
	_delay_us(150);

	cmd_LCD(0x28, 0); // Interface 4 bits, 2 linhas, 5x8 font
	cmd_LCD(0x08, 0); // Display off
	cmd_LCD(0x01, 0); // Clear display
	_delay_ms(2);
	cmd_LCD(0x06, 0); // Entry mode: incrementa cursor
	cmd_LCD(0x0C, 0); // Display on, cursor off, blink off
}


void lcd_clear(void)
{
	cmd_LCD(0x01, 0);
	_delay_ms(2);
}

void lcd_goto(uint8_t linha, uint8_t coluna)
{
	uint8_t addr = coluna + (linha ? 0x40 : 0x00);
	cmd_LCD(0x80 | addr, 0);
}

// Escreve string até o terminador null
void escreve_LCD(char *str) {
	while (*str) {
		cmd_LCD(*str++, 1);
	}
}

// Imprime n?mero com 3 d?gitos
void lcd_print_num(uint8_t val)
{
	char buf[3];
	buf[0] = (val / 100) + '0';
	buf[1] = ((val / 10) % 10) + '0';
	buf[2] = (val % 10) + '0';

	for (int i = 0; i < 3; i++)
	cmd_LCD(buf[i], 1);
}

void set_color_RGB(void);

ISR(TIMER0_COMPA_vect){
	
	if(contagem<100) contagem++;
	
	else passo = 100; 
}

ISR(PCINT1_vect){
	
	if(!(PINC & (1<<BTN_S1))) {
		seletor++;
		if(seletor==3) seletor=-1;
	}
	
	if(!(PINC & (1<<BTN_S2))) {
		incremento = 1;
		TIMSK0 = 0x01;
	}
	
	else
	{
		incremento = 0;
		TIMSK0 = 0x00;
		passo = 1;
		contagem = 0;
	}  

	
	if(!(PINC & (1<<BTN_S3))) {
		decremento = 1;
		TIMSK0 = 0x01;
	}
	else 
	{
		decremento = 0;
		TIMSK0 = 0x00;
		passo = 1;
		contagem = 0;
	}
}
// Bot?es setup e leitura
void buttons_init()
{
	DDRC &= ~((1 << BTN_S1) | (1 << BTN_S2) | (1 << BTN_S3));
	PORTC |= (1 << BTN_S1) | (1 << BTN_S2) | (1 << BTN_S3);
}

void pin_PWM_RGB ()
{
	DDRB |= (1<<PB1)|(1<<PB2)|(1<<PB3);
	
	 // === TIMER1 ===
	 // Modo 14 (Fast PWM, TOP = ICR1), NÃO-invertido, prescaler = 8
	 TCCR1A = 0xA2;
	 TCCR1B = 0x1A;
	 
	 ICR1 = 255; // Limita em 8 bits o valor máximo.
	 
	   // === TIMER2 ===
	   // Modo 5 (Fast PWM com TOP = OCR2A), NÃO-invertido, prescaler = 1
	   
	   TCCR2A = 0x83;
	   TCCR2B = 0x01;
	   
	   //Configuração dos pinos de TIMER 0:
	   TCCR0A = 0x02;
	   TCCR0B = 0x05;
	   TIMSK0 = 0x00;
	   
	   OCR0A = 254;
}
void set_color_RGB()
{
	OCR2A = red_val;      // Define o valor do RG3 - Duty Cycle do led RED
	OCR1B  = green_val;   // Define o valor do RG2 - Duty Cycle do led GREEN
	OCR1A = blue_val;     // Define o valor do RG1 - Duty Cycle do led BLUE
	};



void lcd_update_digit(uint8_t cor_idx, uint8_t val, bool selecionado)
{
	if (cor_idx > 2) return;

	lcd_goto(1, col_base[cor_idx]);

	char buf[3];
	buf[0] = (val / 100) + '0';
	buf[1] = ((val / 10) % 10) + '0';
	buf[2] = (val % 10) + '0';

	for (int i = 0; i < 3; i++)
	cmd_LCD(buf[i], 1);

	cmd_LCD(selecionado ? '*' : ' ', 1);
}

void lcd_update_partial(void)
{
	if (old_red != red_val || old_seletor != seletor)
	{
		lcd_update_digit(0, red_val, (seletor == 0));
		old_red = red_val;
	}
	if (old_green != green_val || old_seletor != seletor)
	{
		lcd_update_digit(1, green_val, (seletor == 1));
		old_green = green_val;
	}
	if (old_blue != blue_val || old_seletor != seletor)
	{
		lcd_update_digit(2, blue_val, (seletor == 2));
		old_blue = blue_val;
	}
	old_seletor = seletor;
}

void controle_de_led()
{
	
	if(incremento==1) {
		switch(seletor){
			case 0:
			if(red_val<255){
				red_val+= passo;
				set_color_RGB();
			}
			break;
			case 1:
			if(green_val<255){
				green_val+= passo;
				set_color_RGB();
			}
			break;
			case 2:
			if(blue_val<255){
				blue_val+= passo;
				set_color_RGB();
			}
			break;
			default:
			break;
		}
		
	}
	
	if(decremento==1) {
		switch(seletor){
			case 0:
			if(red_val>0){
				red_val-= passo;
				set_color_RGB();
			}
			break;
			case 1:
			if(green_val>0){
				green_val-= passo;
				set_color_RGB();
			}
			break;
			case 2:
			if(blue_val>0){
				blue_val-= passo;
				set_color_RGB();
			}
			break;
			default:
			break;
		}
	}
	}

int main(void) {
	buttons_init();
	inic_LCD_4bits();

	lcd_clear();
	lcd_goto(0, 0);
	escreve_LCD("RED  GREEN  BLUE");

	old_red = 255;
	old_green = 255;
	old_blue = 255;
	old_seletor = -2;

	lcd_update_partial();

	PCICR = (1 << PCIE1);
	PCMSK1 |= (1<<PCINT9)|(1<<PCINT10)|(1<<PCINT11);
	
	pin_PWM_RGB();
	
	sei();
	
	while (1)
	{
		lcd_update_partial();
		_delay_ms(100);
		controle_de_led();
		
}
}

