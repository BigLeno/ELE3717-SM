#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "LCD.h"
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
volatile uint8_t sinal_modulado = 0; // sinal modulado
volatile uint8_t cont_aux = 0;

void setup();
void mde(volatile uint16_t msg, volatile uint8_t *sinal_modulado);

int main()
{
    setup();
    ADC_init(); // inicializa o ADC

    inic_LCD_4bits(); // inicializa o LCD

    while (1)
    {
        // if (cont_aux >= 7 && (estado_mde == ESTADO2 || estado_mde == ESTADO3))
        // {
        //     // Apenas para ASK e FSK, segura o valor do ADC por 7 iterações
        //     msg = ler_adc();
        //     if (msg < 15)
        //     { // pequeno tratamento de ruido
        //         msg = 0;
        //     }
        //     cont_aux = 0;
        // }
        msg = ler_adc(); // lê o valor do ADC

        //mde(msg, &sinal_modulado);
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

void mde(volatile uint16_t msg, volatile uint8_t *sinal_modulado)
{
    static char freq_string[4] = {0};
    static char sinal_string[4] = {0};
    static uint8_t freq_portadora = 100; // frequencia da portadora inicial

    switch (estado_mde)
    {
    case ESTADO0:
        cmd_LCD(0x80, 0);           // desloca cursor para a primeira linha
        escreve_LCD("Mod: AM  F:"); // string armazenada na RAM

        freq_portadora += inc_dec;                 // incrementa ou decrementa a frequencia da portadora
        ident_num(freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);                  // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("Hz");
        cmd_LCD(0xC0, 0); // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(*sinal_modulado, sinal_string, 3); // converte o contador para string
        escreve_LCD(sinal_string);                   // escreve o contador no LCD
        break;
    case ESTADO1:
        cmd_LCD(0x80, 0);            // desloca cursor para a primeira linha
        escreve_LCD("Mod: FM  F: "); // string armazenada na RAM

        freq_portadora += inc_dec;                 // incrementa ou decrementa a frequencia da portadora
        ident_num(freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);                  // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("Hz");
        cmd_LCD(0xC0, 0); // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(*sinal_modulado, sinal_string, 3); // converte o contador para string
        escreve_LCD(sinal_string);                   // escreve o contador no LCD
        break;
    case ESTADO2:
        cmd_LCD(0x80, 0);           // desloca cursor para a primeira linha
        escreve_LCD("Mod: ASK T:"); // string armazenada na RAM

        freq_portadora += inc_dec;                 // incrementa ou decrementa a frequencia da portadora
        ident_num(freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);                  // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("bs");
        cmd_LCD(0xC0, 0); // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(*sinal_modulado, sinal_string, 3); // converte o contador para string
        escreve_LCD(sinal_string);                   // escreve o contador no LCD
        break;
    case ESTADO3:
        cmd_LCD(0x80, 0);           // desloca cursor para a primeira linha
        escreve_LCD("Mod: FSK T:"); // string armazenada na RAM

        freq_portadora += inc_dec;                 // incrementa ou decrementa a frequencia da portadora
        ident_num(freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);                  // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("bs");
        cmd_LCD(0xC0, 0); // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(*sinal_modulado, sinal_string, 3); // converte o contador para string
        escreve_LCD(sinal_string);                   // escreve o contador no LCD
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
    
    sinal_modulado = modula_am(msg, cont); // modula FM
    // switch (estado_mde)
    // {
    // case ESTADO0:
    //     sinal_modulado = modula_am(msg, cont); // modula FM
    //     break;
    // case ESTADO1:
    //     sinal_modulado = modula_fm(msg, cont); // modula FM
    //     break;
    // case ESTADO2:
    //     sinal_modulado = modula_ask(msg & 0x01, cont); // modula ASK
    //     break;
    // case ESTADO3:
    //     sinal_modulado = modula_fsk(msg & 0x01, cont); // modula FSK
    //     break;
    // default:
    //     break;
    // }

    cont++;
    if (cont >= 31)
    {
        cont = 0;
        cont_aux++;
        msg = msg >> 1; // desloca a mensagem para a direita, para pegar o proximo bit
    }

    TCNT1 = 0; // reseta o contador do timer
}