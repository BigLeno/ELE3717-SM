#ifndef _LCD_H
#define _LCD_H

#define set_bit(y,bit) (y|=(1<<bit)) //coloca em 1 o bit x da variável Y
#define clr_bit(y,bit) (y&=~(1<<bit)) //coloca em 0 o bit x da variável Y
#define cpl_bit(y,bit) (y^=(1<<bit)) //troca o estado lógico do bit x da variável Y
#define tst_bit(y,bit) (y&(1<<bit)) //retorna 0 ou 1 conforme leitura do bit 

#define DADOS_LCD PORTD//4 bits de dados do LCD no PORTD
#define nibble_dados 1 //0 para via de dados do LCD nos 4 LSBs do PORT

#define CONTR_LCD PORTD//PORT com os pinos de controle do LCD (pino R/W em 0).
#define E PD3 //pino de habilitação do LCD (enable)
#define RS PD2 //pino para informar se o dado é uma instrução ou caractere
#define conv_ascii 48 //48 se ident_num() deve retornar um número no formato ASCII (0 para

//sinal de habilitação para o LCD
#define pulso_enable() _delay_us(1); set_bit(CONTR_LCD,E); _delay_us(1); clr_bit(CONTR_LCD,E); _delay_us(45)

void cmd_LCD(unsigned char c, char cd);
void inic_LCD_4bits();
void escreve_LCD(char *c);
void ident_num(unsigned int valor, char *disp, unsigned int qtd_digit);
#endif 