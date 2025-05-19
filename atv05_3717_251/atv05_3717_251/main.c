#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DATA_PORT_PORT PORTB
#define DATA_PORT_DDR DDRB
#define DIN PB3     // SPI MOSI
#define CS PB2      // Chip select
#define CLK PB5     // SPI Clock

// Inicializa SPI no modo mestre
void spi_init() {
	DATA_PORT_DDR |= (1 << DIN) | (1 << CS) | (1 << CLK);
	// Configura SPI: mestre, clock fosc/16
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
	DATA_PORT_PORT |= (1 << CS); // CS inicial HIGH (desativado)
}

// Transmite 1 byte via SPI
void spi_transmit(uint8_t data) {
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
}

// Envia comando ao MAX7219 (endereço + dado)
void max7219_send(uint8_t addr, uint8_t data) {
	DATA_PORT_PORT &= ~(1 << CS); // CS LOW ativa comunicação
	spi_transmit(addr);
	spi_transmit(data);
	DATA_PORT_PORT |= (1 << CS); // CS HIGH desativa comunicação
}

// Inicializa MAX7219 para uso com matriz 8x8
void max7219_init() {
	max7219_send(0x0F, 0x00); // Display test desligado
	max7219_send(0x09, 0x00); // Decode mode desativado (para matriz)
	max7219_send(0x0A, 0x03); // Intensidade (0-15)
	max7219_send(0x0B, 0x07); // Scan limit = 8 dígitos (linhas)
	max7219_send(0x0C, 0x01); // Normal operation (não shutdown)
}

// Apaga todos os LEDs
void max7219_clear() {
	for (int i = 1; i <= 8; i++) {
		max7219_send(i, 0x00);
	}
}

void max7219_set_column(int col, uint8_t value) {
	max7219_send(col + 1, value);
}

void adc_init() {
	// Configura o ADC para usar referência AVcc (5V) e seleciona canal inicial 0
	ADMUX = (1 << REFS0);
	// Habilita ADC e configura prescaler para 128 (para 16 MHz CPU, ADC ~125 kHz)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// Desliga entradas digitais nos ADC4 e ADC5 (PC4 e PC5) para reduzir interferência
	DIDR0 = (1 << ADC4D) | (1 << ADC5D);
}

uint16_t adc_read(uint8_t channel) {
	// Configurar e iniciar ADC, ajustar para o canal indicado (0 a 7)
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1<<ADSC);                     // Inicia conversão
	while(ADCSRA & (1<<ADSC));               // Aguarda conversão terminar
	return ADCW;
}



int main() {
	spi_init();
	max7219_init();
	max7219_clear();
	adc_init();     // Chame aqui a inicialização do ADC

	int pos_x = 0, pos_y = 0;
	int last_pos_x = 0, last_pos_y = 0;

	while(1) {
		uint16_t adc_x = adc_read(4); // PC4 -> ADC4
		uint16_t adc_y = adc_read(5); // PC5 -> ADC5

		pos_x = (adc_x * 8) / 1024;
		pos_y = (adc_y * 8) / 1024;

		if(pos_x > 7) pos_x = 7;
		if(pos_y > 7) pos_y = 7;

		// Apaga o ponto anterior
		max7219_send(last_pos_y + 1, 0x00);

		// Acende o ponto novo
		max7219_send(pos_y + 1, 1 << pos_x);

		last_pos_x = pos_x;
		last_pos_y = pos_y;

		_delay_ms(50);
	}
}