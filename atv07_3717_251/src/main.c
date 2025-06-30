
// --- INCLUDES PRINCIPAIS ---
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "lcd.h"
#include "adc.h"
#include "modulador.h"
#include "btn.h"

// Prototipação das funções usadas antes do main
void lcd_print_bin(uint8_t num);
void mde(uint16_t msg, volatile uint8_t *sinal_modulado);

// Função para exibir um número em binário no LCD (8 bits)
void lcd_print_bin(uint8_t num) {
    char str[9];
    for (int i = 7; i >= 0; i--) {
        str[7-i] = (num & (1 << i)) ? '1' : '0';
    }
    str[8] = '\0';
    lcd_print(str);
}
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "lcd.h"
#include "ADC.h"
#include "modulador.h"

#define ESTADO0 0
#define ESTADO1 1
#define ESTADO2 2
#define ESTADO3 3

volatile int8_t inc_dec = 0; // -1 = decremento, 1 = incremento, 0 = nada
volatile uint8_t estado_mde = ESTADO0;
volatile uint16_t cont = 0;          // contador de tempo
volatile uint16_t msg = 0;           // mensagem a ser enviada
uint8_t sinal_modulado = 0; // sinal modulado
volatile uint8_t cont_aux = 0;

void setup();
// Removido: declaração antiga incompatível

int main()
{

    setup();
    adc_init(); // inicializa o ADC
    lcd_init(); // inicializa o LCD
    btn_init(); // inicializa os botões

    while (1)
    {
        if (cont_aux >= 7 && (estado_mde == ESTADO2 || estado_mde == ESTADO3))
        {
            // Apenas para ASK e FSK, segura o valor do ADC por 7 iterações
            msg = adc_read_a0();
            if (msg < 15)
            { // pequeno tratamento de ruido
                msg = 0;
            }
            cont_aux = 0;
        }
        msg = adc_read_a0(); // lê o valor do ADC

        mde(msg, &sinal_modulado);
        envia_dados(sinal_modulado); // envia o sinal modulado para o DAC
    }
}

void setup()
{
    DDRC = 0x00;
    DDRC &= ~(1 << PC0); // entrada x
    DDRC &= ~0x0E;       // setando PC1 a PC3 como entrada, BTNS

    DDRD = 0x00;
    DDRD |= 0xFC; // setando PD2, PD3 PD4 a PD7 como saida, LCD

    DDRC |= 0x30; // setando PC5 a PC6 como saida, coef (DA0, DA1)
    DDRB = 0x3F;  // setando PB0 a PB5 como saida, coef (DA2 - DA7)

    PCICR = 0x02; // interrupcao btn
    PCMSK1 = 0x0E;

    TCCR1A = 0x00; // modo ctc
    TCCR1B = 0x0A; // prescaler de 8
    TCNT1 = 0;
    TIMSK1 = (1 << OCIE1A); // HABILITA INTERRUPCAO COM COMPARADOR COM A
    OCR1A = 600;            // 600 - PROXIMO DE 100HZ, 30000 É 33 hz, 1000 É 1000hz
    sei();
}

void mde(uint16_t msg, volatile uint8_t *sinal_modulado)
{
    static uint8_t freq_portadora = 100; // frequencia da portadora inicial

    switch (estado_mde)
    {
    case ESTADO0:
        lcd_goto(0, 0);           // primeira linha
        lcd_print("Mod: AM  F:");
        freq_portadora = (uint8_t)((int16_t)freq_portadora + inc_dec);
        if (freq_portadora < 100) freq_portadora = 100;
        if (freq_portadora > 231) freq_portadora = 231; // 231 é o maior valor possível em uint8_t abaixo de 999
        lcd_print_dec(freq_portadora);
        lcd_goto(0, 14);
        lcd_print("Hz");
        lcd_goto(1, 0); // segunda linha
        lcd_print("Msg: ");
        lcd_print_dec(*sinal_modulado);
        break;
    case ESTADO1:
        lcd_goto(0, 0);
        lcd_print("Mod: FM  F: ");
        freq_portadora = (uint8_t)((int16_t)freq_portadora + inc_dec);
        if (freq_portadora < 100) freq_portadora = 100;
        if (freq_portadora > 231) freq_portadora = 231;
        lcd_print_dec(freq_portadora);
        lcd_goto(0, 14);
        lcd_print("Hz");
        lcd_goto(1, 0);
        lcd_print("Msg: ");
        lcd_print_dec(*sinal_modulado);
        break;
    case ESTADO2:
        lcd_goto(0, 0);
        lcd_print("Mod: ASK T:");
        freq_portadora = (uint8_t)((int16_t)freq_portadora + inc_dec);
        if (freq_portadora < 100) freq_portadora = 100;
        if (freq_portadora > 231) freq_portadora = 231;
        lcd_print_dec(freq_portadora);
        lcd_goto(0, 14);
        lcd_print("bs");
        lcd_goto(1, 0);
        lcd_print("Msg: ");
        lcd_print_bin(*sinal_modulado);
        break;
    case ESTADO3:
        lcd_goto(0, 0);
        lcd_print("Mod: FSK T:");
        freq_portadora = (uint8_t)((int16_t)freq_portadora + inc_dec);
        if (freq_portadora < 100) freq_portadora = 100;
        if (freq_portadora > 231) freq_portadora = 231;
        lcd_print_dec(freq_portadora);
        lcd_goto(0, 14);
        lcd_print("bs");
        lcd_goto(1, 0);
        lcd_print("Msg: ");
        lcd_print_bin(*sinal_modulado);
        break;
    default:
        break;
    }
}

ISR(PCINT1_vect)
{
    if (PINC == 0x06)
    {
        //	S3 pressionado (M)
        estado_mde++;
        if (estado_mde > ESTADO3)
        {
            estado_mde = ESTADO0; // volta ao estado inicial
        }
    }
    else if (PINC == 0x0A)
    {
        // S2 pressionado (+)
        inc_dec = 1; // seta incremento
    }
    else if (PINC == 0x0C)
    {
        // S1 pressionado (-)
        inc_dec = -1;
    }
    else
    {
        inc_dec = 0; // limpa o incremento/decremento
    }
}

ISR(TIMER1_COMPA_vect)
{
    switch (estado_mde)
    {
    case ESTADO0:
        sinal_modulado = modula_am(msg, cont); // modula FM
        break;
    case ESTADO1:
        sinal_modulado = modula_fm(msg, cont); // modula FM
        break;
    case ESTADO2:
        sinal_modulado = modula_ask(msg & 0x01, cont); // modula ASK
        break;
    case ESTADO3:
        sinal_modulado = modula_fsk(msg & 0x01, cont); // modula FSK
        break;
    default:
        break;
    }

    cont++;
    if (cont >= 31)
    {
        cont = 0;
        cont_aux++;
        msg = msg >> 1; // desloca a mensagem para a direita, para pegar o proximo bit
    }

    TCNT1 = 0; // reseta o contador do timer
}