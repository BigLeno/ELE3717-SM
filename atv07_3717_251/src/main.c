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
volatile uint16_t cont = 0;     // contador de tempo
volatile uint8_t flag_update_lcd = 0;
volatile uint16_t timer_lcd_count = 0;
volatile uint8_t sinal_modulado = 0; // sinal modulado

void setup();
void mde(uint16_t *freq_portadora, uint16_t msg, volatile uint8_t *sinal_modulado);

int main()
{
    setup();
    ADC_init(); // inicializa o ADC

    inic_LCD_4bits(); // inicializa o LCD

    uint16_t msg = 0; // mensagem a ser enviada
    uint16_t freq_portadora = 100; // frequencia inicial da portadora

    while (1)
    {
        // Atualiza modulação em alta frequência
        ler_adc(&msg); // le o valor do ADC e armazena em msg
        
        // Chama a modulação apropriada baseada no estado
        switch (estado_mde) {
            case ESTADO0:
                sinal_modulado = modula_am(freq_portadora, msg);
                break;
            case ESTADO1:
                sinal_modulado = modula_fm(freq_portadora, msg);
                break;
            case ESTADO2:
                sinal_modulado = modula_ask(freq_portadora, msg);
                break;
            case ESTADO3:
                sinal_modulado = modula_fsk(freq_portadora, msg);
                break;
        }
        
        envia_dados(sinal_modulado); // envia o sinal modulado para o DAC
        
        // Atualiza LCD apenas quando necessário
        if (flag_update_lcd) {
            mde(&freq_portadora, msg, &sinal_modulado);
            flag_update_lcd = 0;
        }
        
        _delay_us(45); // delay para 20 kHz de taxa de amostragem (50μs total incluindo processamento)
    }
}

void setup()
{
    // Configuração do PORTC
    DDRC = 0x00;               // Limpa todas as configurações
    DDRC &= ~(1 << PC0);       // PC0 como entrada (ADC)
    DDRC &= ~0x0E;             // PC1, PC2, PC3 como entrada (botões)
    DDRC |= 0x30;              // PC4, PC5 como saída (DAC bits 0-1)
    
    // Pull-up nos botões (apenas PC1, PC2, PC3)
    PORTC = (PORTC & 0xF1) | 0x0E;  // Habilita pull-up nos botões sem afetar DAC
    
    // Configuração do PORTD (LCD)
    DDRD = 0x00; 
    DDRD |= 0xFC;              // PD2, PD3, PD4-PD7 como saída (LCD)

    // Configuração do PORTB (DAC bits 2-7)
    DDRB = 0x3F;               // PB0-PB5 como saída (DAC)

    // Configuração das interrupções dos botões
    PCICR = 0x02;              // Habilita interrupção PCINT1
    PCMSK1 = 0x0E;             // Máscara para PC1, PC2, PC3

    // Configuração do Timer1 para controle do LCD
    TCCR1A = 0x00;             // Modo CTC
    TCCR1B = 0x0A;             // Prescaler de 8
    TCNT1 = 0;
    TIMSK1 = (1 << OCIE1A);    // Habilita interrupção de comparação A
    OCR1A = 10000;             // Aprox. 10Hz para atualização do LCD (16MHz/8/10000)
    
    sei();                     // Habilita interrupções globais
}

void mde(uint16_t *freq_portadora, uint16_t msg, volatile uint8_t *sinal_modulado)
{
    static char freq_string[4] = {0}; 
    static char sinal_string[4] = {0}; 
    static uint8_t estado_anterior = 255; // para detectar mudança de estado

    // Limpa display apenas na mudança de estado
    if (estado_mde != estado_anterior) {
        cmd_LCD(0x01, 0);              // limpa o display
        estado_anterior = estado_mde;
    }

    switch (estado_mde)
    {
    case ESTADO0:
        cmd_LCD(0x80, 0);              // desloca cursor para a primeira linha
        escreve_LCD("Mod: AM  F:"); // string armazenada na RAM

        *freq_portadora += inc_dec; // incrementa ou decrementa a frequencia da portadora
        if (*freq_portadora < 50) *freq_portadora = 50;   // limite mínimo
        if (*freq_portadora > 999) *freq_portadora = 999; // limite máximo
        
        ident_num(*freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);            // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("Hz");
        cmd_LCD(0xC0, 0);              // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(msg, sinal_string, 3); // mostra o valor da mensagem
        escreve_LCD(sinal_string);            // escreve no LCD
        break;
    case ESTADO1:
        cmd_LCD(0x80, 0);              // desloca cursor para a primeira linha
        escreve_LCD("Mod: FM  F: "); // string armazenada na RAM

        *freq_portadora += inc_dec; // incrementa ou decrementa a frequencia da portadora
        if (*freq_portadora < 50) *freq_portadora = 50;   // limite mínimo
        if (*freq_portadora > 999) *freq_portadora = 999; // limite máximo
        
        ident_num(*freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);            // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("Hz");
        cmd_LCD(0xC0, 0);              // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(msg, sinal_string, 3); // mostra o valor da mensagem
        escreve_LCD(sinal_string);            // escreve no LCD
        break;
    case ESTADO2:
        cmd_LCD(0x80, 0);              // desloca cursor para a primeira linha
        escreve_LCD("Mod: ASK T:"); // string armazenada na RAM

        *freq_portadora += inc_dec; // incrementa ou decrementa a frequencia da portadora
        if (*freq_portadora < 50) *freq_portadora = 50;   // limite mínimo
        if (*freq_portadora > 999) *freq_portadora = 999; // limite máximo
        
        ident_num(*freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);            // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("bs");
        cmd_LCD(0xC0, 0);              // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(msg, sinal_string, 3); // mostra o valor da mensagem
        escreve_LCD(sinal_string);            // escreve no LCD
        break;
    case ESTADO3:
        cmd_LCD(0x80, 0);              // desloca cursor para a primeira linha
        escreve_LCD("Mod: FSK T:"); // string armazenada na RAM

        *freq_portadora += inc_dec; // incrementa ou decrementa a frequencia da portadora
        if (*freq_portadora < 50) *freq_portadora = 50;   // limite mínimo
        if (*freq_portadora > 999) *freq_portadora = 999; // limite máximo
        
        ident_num(*freq_portadora, freq_string, 3); // converte o contador para string
        escreve_LCD(freq_string);            // escreve o contador no LCD

        cmd_LCD(0x8E, 0);
        escreve_LCD("bs");
        cmd_LCD(0xC0, 0);              // desloca cursor para a segunda linha
        escreve_LCD("Msg: ");

        ident_num(msg, sinal_string, 3); // mostra o valor da mensagem
        escreve_LCD(sinal_string);            // escreve no LCD
        break;
    default:
        break;
    }
    
    inc_dec = 0; // reseta o incremento/decremento após usar
}

ISR(PCINT1_vect)
{
    _delay_ms(50); // debounce
    
    uint8_t botoes = PINC & 0x0E; // lê apenas os bits dos botões
    
    if (~botoes & (1 << PC1)) // S1 pressionado (-), lógica invertida devido ao pull-up
    {
        inc_dec = -1;
    }
    else if (~botoes & (1 << PC2)) // S2 pressionado (+)
    {
        inc_dec = 1;
    }
    else if (~botoes & (1 << PC3)) // S3 pressionado (M)
    {
        estado_mde++;
        if (estado_mde > ESTADO3)
        {
            estado_mde = ESTADO0; // volta ao estado inicial
        }
    }
}

ISR(TIMER1_COMPA_vect)
{
    // Sinaliza para atualizar o LCD
    flag_update_lcd = 1;
    TCNT1 = 0; // reseta o contador do timer
}