/*
 * atv08_3717_251.c
 *
 * Created: 14/07/2025 15:51:50
 * Author : livia
 */ 
#define F_CPU 16000000UL

//***BIBLIOTECAS***//
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"
#include <stdbool.h>
#include <stdio.h>

//***DEFINI��ES DE PINOS PARA BOT�ES (conectados a PORTC)***//
#define BTN_DOWN    PINC0   // Liga/desliga sa�da
#define BTN_M    PINC1   // Muda tipo de onda
#define BTN_UP PINC2   // Diminui frequ�ncia
#define BTN_A   PINC3   // Aumenta frequ�ncia

//**ENUMERA��O DAS ONDAS**//
typedef enum {Quadrada, Triangular, Rampa, Senoide, Total_ondas} TipoOnda;

//**Tamanho da Vetor**//
#define TAM_VEC 33

//** VARI�VEL DE CONTROLE DO DUTY CYCLE**//
volatile uint8_t valor_comp_dc = 0;
 
//**CRIA��O DO VETOR TABELA DO SENO**//
const uint8_t onda_seno[TAM_VEC] = {
	127, 151, 175, 197, 216, 232, 244, 251, 254, 251, 244, 232, 216, 197, 175, 151, 127, 102, 78, 56, 37, 21, 9, 2, 0, 2, 9, 21, 37, 56, 78, 102, 127
};

//***VARI�VEIS GLOBAIS***//
volatile TipoOnda onda_selecionada = Quadrada;
volatile uint8_t duty_cycle = 50;
volatile uint16_t freq_hz = 10;
volatile uint8_t amp_vpp = 102;
volatile uint8_t offset_v = 128;
volatile bool saida_ligada = false;
volatile uint8_t idx_tabela = 0;

volatile uint8_t incremento = 0;
volatile uint8_t decremento = 0;
volatile uint8_t passo_rampa = 8;
volatile uint8_t vout_rampa = 0;
volatile uint8_t vout_tri = 0;

volatile int8_t contador = -1; // -1 =DESATIVADO, 0=QUADRADA, 1=TRIANGULAR, 2=RAMPA, 3=SENOIDE

//***DEFINI��ES PARA LCD 4 bits em PORTD***//
#define PORT_LCD PORTD
#define DDR_LCD  DDRD
#define RS PD2
#define EN PD3
#define LCD_DATA_MASK ((1 << PD4)|(1 << PD5)|(1 << PD6)|(1 << PD7))

#define set_bit(Y, bit_x)  (Y |= (1 << (bit_x)))
#define clr_bit(Y, bit_x)  (Y &= ~(1 << (bit_x)))

//**FUN��O DE AJUSTE DO DUTY CYCLE**//
uint8_t ajuste_dc(uint8_t dc){
	
	return ((dc/100.0)*31);
}

//**FUN��O DE AJUSTE DE FREQU�NCIA**//

uint16_t ajuste_freq(uint16_t f){
	
return (62500/f);
}

//**FUN��O DOS BOT�ES**//
void buttons_init()
{
	DDRC = 0X30;
	//PORTC |= (1 << BTN_DOWN) | (1 << BTN_M) | (1 << BTN_UP) | (1 << BTN_A);
}


//***FUN��ES DO LCD***//

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
	// Inicializa��o especial modo 4 bits (sequ�ncia do datasheet)
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

// Escreve string at� o terminador null
void escreve_LCD(char *str) {
	while (*str) {
		cmd_LCD(*str++, 1);
	}
}

// Imprime n�mero com 3 d�gitos
void lcd_print_num(uint8_t val)
{
	char buf[3];
	buf[0] = (val / 100) + '0';
	buf[1] = ((val / 10) % 10) + '0';
	buf[2] = (val % 10) + '0';

	for (int i = 0; i < 3; i++)
	cmd_LCD(buf[i], 1);
}

//**INTERRUP��O PARA GERA��O DOS SINAIS
ISR(TIMER1_COMPA_vect){
 if(saida_ligada==true){
	switch(onda_selecionada){
		case  Quadrada:
		 if (valor_comp_dc >= idx_tabela)
		 {
			 PORTB = (0xFF >> 2);
			 PORTC = (0xFF & 0x03 << 4);
		 }
		 else {
			 PORTB = (0x00 >> 2);
			 PORTC = (0x00 & 0x03 << 4);
		 }
		break;
		case Triangular:
		if (idx_tabela <= valor_comp_dc)
		{
			vout_tri = vout_tri + (255./(valor_comp_dc+1));
			PORTB = (vout_tri >> 2);
			PORTC = (vout_tri & 0x03 << 4);
		}
		else {
			vout_tri = vout_tri - (255./(33-valor_comp_dc));
			PORTB = (vout_tri >> 2);
			PORTC = (vout_tri & 0x03 << 4);
		}
		break;
		case Rampa:
		vout_rampa = vout_rampa+passo_rampa;
		 PORTB = (vout_rampa >> 2);
		 PORTC = (vout_rampa & 0x03 << 4);
		break;
		case Senoide:
		PORTB = (onda_seno[idx_tabela]  >> 2);
		PORTC = (onda_seno[idx_tabela] & 0x03) << 4;
		break;
		case Total_ondas:
		// Não faz nada, apenas para evitar warning
		break;
	}
	
	idx_tabela++;
	if (idx_tabela>31)
	{
		idx_tabela = 0;
		vout_tri = 0;
	}
 }
 else {
	 PORTB = (128  >> 2);
	 PORTC = (128 & 0x03) << 4;
 }
}

//**INTERRUP��O DE ACIONAMENTO DOS BOT�ES**//
ISR(PCINT1_vect){
	
	if(!(PINC & (1<<BTN_A))) {
		saida_ligada = !saida_ligada;
	}
	
	if(!(PINC & (1<<BTN_M))){
		contador++;
		switch(contador){
			case 0: 
			onda_selecionada = Quadrada;
			break;
			
			case 1:
			onda_selecionada = Triangular;
			break;
			
			case 2:
			onda_selecionada = Rampa;
			break;
			
			case 3:
			onda_selecionada = Senoide;
			break;
			
			default:
			contador = -1;
			break;
		}
	}
	
	taskYIELD();
	
}

//**TASK DE CONTROLE DO LCD**//
void vTaskLCD (void *pv){

inic_LCD_4bits();
	
	while(1)
	{
		lcd_clear();
		lcd_goto(0, 0);
		escreve_LCD("hello");
	
		vTaskDelay(pdMS_TO_TICKS(5000));	
	}
}


int main(void)
{
	buttons_init();

	PCICR = (1 << PCIE1);
	PCMSK1 = 0x0F;
	//PCMSK1 |= (1<<PCINT9)|(1<<PCINT10)|(1<<PCINT11);

	DDRB = 0x3F;
	DDRC = 0x30;
	TCCR1A = 0x00;
	TCCR1B = 0x0A;
	TCCR1C = 0x00;
	TCNT1  = 0;
	TIMSK1 = (1<<OCIE1A);
	OCR1A  = ajuste_freq(freq_hz);
	sei();

	valor_comp_dc = ajuste_dc(duty_cycle);

	inic_LCD_4bits();

	// xTaskCreate retorna BaseType_t, pode ser ignorado se não for usado
	(void)xTaskCreate(vTaskLCD, "LCD", 128, NULL, 1, NULL);
	vTaskStartScheduler();

	// O loop abaixo nunca será alcançado, mas evita warning de função sem retorno
	for(;;) {}
}

