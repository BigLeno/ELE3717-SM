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

int main() {
	spi_init();
	max7219_init();

	while (1) {
		// 1) Acender colunas do 0 ao 7 (todas as linhas com bit da coluna ligado)
		// Vai da coluna 0 até 7
		for (int col = 0; col < 8; col++) {
			uint8_t val = 1 << col;
			for (int linha = 1; linha <= 8; linha++) {
				max7219_send(linha, val);
			}
			_delay_ms(200);
		}

		// Volta da coluna 6 até 1 (não repete coluna 7 e 0)
		for (int col = 6; col > 0; col--) {
			uint8_t val = 1 << col;
			for (int linha = 1; linha <= 8; linha++) {
				max7219_send(linha, val);
			}
			_delay_ms(200);
		}

		// 2) Linha acesa sobe (linha 1 até 8), coluna fixa na última coluna acesa
		//int col_fixa = 7;  // por exemplo, usa a última coluna acesa
		//uint8_t val = 1 << col_fixa;
		for (int linha = 1; linha <= 8; linha++) {
			for (int i = 1; i <= 8; i++) {
				if (i == linha) {
					max7219_send(i, 0xFF);  // linha inteira acesa
					} else {
					max7219_send(i, 0x00);  // outras linhas apagadas
				}
			}
			_delay_ms(200);
		}

		// 3) Linha acesa desce (linha 8 até 1), mesma coluna fixa
		for (int linha = 8; linha >= 1; linha--) {
			for (int i = 1; i <= 8; i++) {
				if (i == linha) {
					max7219_send(i, 0xFF);  // linha inteira acesa
					} else {
					max7219_send(i, 0x00);  // outras linhas apagadas
				}
			}
			_delay_ms(200);
		}
	}
	return 0;
}