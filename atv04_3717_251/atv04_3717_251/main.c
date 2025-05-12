#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/*================================================================
  Configuração de hardware
  ---------------------------------------------------------------
  BOTÕES:
    M    -> PD2  (INT0 e PCINT18)
    ?    -> PD3  (INT1 e PCINT19)
    ?    -> PD4  (       PCINT20)

  LED RGB:
    RED   -> OC0A (PD6)   Timer0 Fast PWM 8-bit
    GREEN -> OC1A (PB1)   Timer1 Fast PWM 8-bit (modo 3)
    BLUE  -> OC2A (PB3)   Timer2 Fast PWM 8-bit

  LCD 2×16 em modo 4-bit (porta C):
    RS -> PC0
    RW -> PC1
    EN -> PC2
    D4 -> PC3
    D5 -> PC4
    D6 -> PC5
    D7 -> PC6
  ================================================================*/
 
// pinos de botão
#define BTN_M     PIND2
#define BTN_UP    PIND3
#define BTN_DOWN  PIND4

// pinos LCD
#define LCD_RS    PC0
#define LCD_RW    PC1
#define LCD_EN    PC2
#define LCD_D4    PC3
#define LCD_D5    PC4
#define LCD_D6    PC5
#define LCD_D7    PC6

// constantes de tempo (em ms)
#define DEBOUNCE_MS     50
#define HOLD_THR1     5000   // auto-repeat após 5s
#define HOLD_THR2    10000   // acelera após 10s
#define REP_INT1       500   // intervalo lento (500ms)
#define REP_INT2       100   // intervalo rápido (100ms)

// variáveis de sistema
volatile uint8_t red_val   = 128;
volatile uint8_t grn_val   = 128;
volatile uint8_t blu_val   = 128;
volatile uint8_t channel   = 0;    // 0=R,1=G,2=B
volatile bool    lcd_update = false;

// contador global de ms (systick)
volatile uint32_t systick_ms = 0;

// estados dos botões
typedef struct {
    bool     pressed;
    uint32_t last_evt;     // timestamp da última transição válida
    uint32_t hold_time;    // tempo que está pressionado
    bool     autorep;      // entrou em modo auto-repeat?
    uint16_t rpt_cnt;      // contagem interna para repetição
} btn_t;

volatile btn_t btnM   = {0}, btnUP = {0}, btnDN = {0};

/*================================================================
  Protótipos
  ================================================================*/
void pwm_init(void);
void pwm_update(void);
void lcd_init(void);
void lcd_cmd(uint8_t c);
void lcd_data(uint8_t d);
void lcd_goto(uint8_t linha, uint8_t col);
void lcd_print(char *s);
void lcd_print16(char *s);  // até 16 chars
void lcd_refresh(void);

/*================================================================
  Interrupção do SysTick (Timer1 CTC A) a cada 1 ms
  ---------------------------------------------------------------
  - Incrementa systick_ms
  - Tratamento de auto-repeat de ?/?
  ================================================================*/
ISR(TIMER1_COMPA_vect)
{
    systick_ms++;
    // ?
    if (btnUP.pressed) {
        btnUP.hold_time++;
        if (btnUP.hold_time >= HOLD_THR1) {
            btnUP.autorep = true;
            btnUP.rpt_cnt++;
            uint16_t thr = (btnUP.hold_time < HOLD_THR2 ? REP_INT1 : REP_INT2);
            if (btnUP.rpt_cnt >= thr) {
                btnUP.rpt_cnt = 0;
                // dispara incremento
                if (channel==0 && red_val<255)   red_val++;
                if (channel==1 && grn_val<255)   grn_val++;
                if (channel==2 && blu_val<255)   blu_val++;
                pwm_update();
                lcd_update = true;
            }
        }
    }
    // ?
    if (btnDN.pressed) {
        btnDN.hold_time++;
        if (btnDN.hold_time >= HOLD_THR1) {
            btnDN.autorep = true;
            btnDN.rpt_cnt++;
            uint16_t thr = (btnDN.hold_time < HOLD_THR2 ? REP_INT1 : REP_INT2);
            if (btnDN.rpt_cnt >= thr) {
                btnDN.rpt_cnt = 0;
                // dispara decremento
                if (channel==0 && red_val>0)   red_val--;
                if (channel==1 && grn_val>0)   grn_val--;
                if (channel==2 && blu_val>0)   blu_val--;
                pwm_update();
                lcd_update = true;
            }
        }
    }
}

/*================================================================
  Interrupção de PCINT2: PORTD[7:0], usamos PD2,PD3,PD4
  ---------------------------------------------------------------
  - Detecta bordas em M, ?, ?
  - Debounce via software (ignora eventos em <DEBOUNCE_MS)
  - Nó: botões são ativos-baixo (pull-up interna)
  ================================================================*/
ISR(PCINT2_vect)
{
    uint8_t pd = PIND;
    uint32_t t = systick_ms;

    // --- botão M (PD2) ---
    if (!(pd & (1<<BTN_M))) {
        // detecta pressão
        if (!btnM.pressed && (t - btnM.last_evt) >= DEBOUNCE_MS) {
            btnM.pressed    = true;
            btnM.last_evt   = t;
            // avança canal
            channel = (channel+1) % 3;
            lcd_update = true;
        }
    } else {
        // detecta liberação
        if (btnM.pressed && (t - btnM.last_evt) >= DEBOUNCE_MS) {
            btnM.pressed  = false;
            btnM.last_evt = t;
        }
    }

    // --- botão ? (PD3) ---
    if (!(pd & (1<<BTN_UP))) {
        if (!btnUP.pressed && (t - btnUP.last_evt) >= DEBOUNCE_MS) {
            btnUP.pressed    = true;
            btnUP.last_evt   = t;
            btnUP.hold_time  = 0;
            btnUP.autorep    = false;
            btnUP.rpt_cnt    = 0;
            // primeiro incremento imediato
            if (channel==0 && red_val<255)   red_val++;
            if (channel==1 && grn_val<255)   grn_val++;
            if (channel==2 && blu_val<255)   blu_val++;
            pwm_update();
            lcd_update = true;
        }
    } else {
        if (btnUP.pressed && (t - btnUP.last_evt) >= DEBOUNCE_MS) {
            btnUP.pressed    = false;
            btnUP.last_evt   = t;
            btnUP.hold_time  = 0;
            btnUP.autorep    = false;
            btnUP.rpt_cnt    = 0;
        }
    }

    // --- botão ? (PD4) ---
    if (!(pd & (1<<BTN_DOWN))) {
        if (!btnDN.pressed && (t - btnDN.last_evt) >= DEBOUNCE_MS) {
            btnDN.pressed    = true;
            btnDN.last_evt   = t;
            btnDN.hold_time  = 0;
            btnDN.autorep    = false;
            btnDN.rpt_cnt    = 0;
            // primeiro decremento imediato
            if (channel==0 && red_val>0)   red_val--;
            if (channel==1 && grn_val>0)   grn_val--;
            if (channel==2 && blu_val>0)   blu_val--;
            pwm_update();
            lcd_update = true;
        }
    } else {
        if (btnDN.pressed && (t - btnDN.last_evt) >= DEBOUNCE_MS) {
            btnDN.pressed    = false;
            btnDN.last_evt   = t;
            btnDN.hold_time  = 0;
            btnDN.autorep    = false;
            btnDN.rpt_cnt    = 0;
        }
    }
}

/*================================================================
  Inicialização de PWM nos timers 0,1,2
  ================================================================*/
void pwm_init(void)
{
    // Timer0 Fast PWM 8bit – RED – OC0A = PD6
    DDRD |= (1<<PD6);
    TCCR0A = (1<<WGM00)|(1<<WGM01)|(1<<COM0A1);  // Fast PWM, non-inverting
    TCCR0B = (1<<CS01)|(1<<CS00);               // prescaler 64

    // Timer1 Fast PWM 8bit – GREEN – OC1A = PB1
    DDRB |= (1<<PB1);
    TCCR1A = (1<<WGM10)|(1<<WGM11)|(1<<COM1A1);
    TCCR1B = (1<<CS11)|(1<<CS10);  // Fast PWM 8bit, prescaler 64

    // Timer2 Fast PWM 8bit – BLUE – OC2A = PB3
    DDRB |= (1<<PB3);
    TCCR2A = (1<<WGM20)|(1<<WGM21)|(1<<COM2A1);
    TCCR2B = (1<<CS22);            // prescaler 64

    pwm_update();
}

/* atualiza valores PWM */
void pwm_update(void)
{
    OCR0A = red_val;
    OCR1AL= grn_val;
    OCR2A = blu_val;
}

/*================================================================
  Rotina de inicialização do SysTick (Timer1 CTC A – 1 ms)
  ---------------------------------------------------------------
  Usamos Timer1 em CTC A, OCR1A = 249 @ prescaler=64 em 16 MHz
  => 1 ms de interrupção
  ================================================================*/
void systick_init(void)
{
    TCCR1A &= ~((1<<WGM11)|(1<<WGM10));  // WGM1[3:2]=00
    TCCR1B = (1<<WGM12)|(1<<CS11)|(1<<CS10); // CTC, prescaler 64
    OCR1A = 249;
    TIMSK1 |= (1<<OCIE1A);
}

/*================================================================
  Inicialização PCINT para PD2–PD4
  ================================================================*/
void pcint_init(void)
{
    PCICR |= (1<<PCIE2);      // habilita PCINT2 (PORTD)
    PCMSK2 |= (1<<PCINT18)|(1<<PCINT19)|(1<<PCINT20);
}

/*================================================================
  Funções básicas do LCD 2×16 em 4-bit
  ================================================================*/
static void lcd_strobe(void)
{
    PORTC |=  (1<<LCD_EN);
    _delay_us(1);
    PORTC &= ~(1<<LCD_EN);
    _delay_us(100);
}

/* envia meio byte */
static void lcd_write_nibble(uint8_t nib)
{
    if (nib & 0x08) PORTC |=  (1<<LCD_D7); else PORTC &= ~(1<<LCD_D7);
    if (nib & 0x04) PORTC |=  (1<<LCD_D6); else PORTC &= ~(1<<LCD_D6);
    if (nib & 0x02) PORTC |=  (1<<LCD_D5); else PORTC &= ~(1<<LCD_D5);
    if (nib & 0x01) PORTC |=  (1<<LCD_D4); else PORTC &= ~(1<<LCD_D4);
    lcd_strobe();
}

/* envia comando RS=0 */
void lcd_cmd(uint8_t c)
{
    PORTC &= ~(1<<LCD_RS);
    _delay_us(1);
    lcd_write_nibble(c>>4);
    lcd_write_nibble(c&0x0F);
    _delay_ms(2);
}

/* envia dado RS=1 */
void lcd_data(uint8_t d)
{
    PORTC |=  (1<<LCD_RS);
    _delay_us(1);
    lcd_write_nibble(d>>4);
    lcd_write_nibble(d&0x0F);
    _delay_us(40);
}

/* posiciona cursor (linha 0/1, coluna 0–15) */
void lcd_goto(uint8_t linha, uint8_t col)
{
    uint8_t addr = (linha==0 ? 0x00 : 0x40) + col;
    lcd_cmd(0x80 | addr);
}

/* imprime string terminada em ‘\0’ */
void lcd_print(char *s)
{
    while (*s) { lcd_data(*s++); }
}

/* atualiza toda a tela com valores e “*” no canal ativo */
void lcd_refresh(void)
{
    char buf1[17], buf2[17];
    // formata linha 1: *R:XXX G:XXX
    snprintf(buf1,16, "%cR:%3u G:%3u",
             (channel==0?'*':' '),
              red_val, grn_val);
    // formata linha 2: *B:XXX
    snprintf(buf2,16, "%cB:%3u",
             (channel==2?'*':' '),
             blu_val);
    // desenha
    lcd_cmd(0x01);        // limpa tela
    _delay_ms(2);
    lcd_goto(0,0);
    lcd_print(buf1);
    lcd_goto(1,0);
    lcd_print(buf2);
}

/* setup inicial do LCD */
void lcd_init(void)
{
    // portas como saída
    DDRC |= (1<<LCD_RS)|(1<<LCD_RW)|(1<<LCD_EN)
          |(1<<LCD_D4)|(1<<LCD_D5)|(1<<LCD_D6)|(1<<LCD_D7);
    _delay_ms(40);
    // init sequence 4-bit
    PORTC &= ~(1<<LCD_RW);
    lcd_strobe(); lcd_strobe(); lcd_strobe();
    lcd_write_nibble(0x02);
    lcd_cmd(0x28);  // 4-bit, 2 linhas, fonte 5×8
    lcd_cmd(0x08);  // display off
    lcd_cmd(0x0C);  // display on, cursor off
    lcd_cmd(0x06);  // entrada incrementa
    lcd_cmd(0x01);  // limpa
    _delay_ms(2);
}

/*================================================================
  Função principal
  ================================================================*/
int main(void)
{
    // desabilita interrupções
    cli();
    // pull-ups em PD2,PD3,PD4
    DDRD &= ~((1<<BTN_M)|(1<<BTN_UP)|(1<<BTN_DOWN));
    PORTD |=  ((1<<BTN_M)|(1<<BTN_UP)|(1<<BTN_DOWN));

    pwm_init();
    lcd_init();
    systick_init();
    pcint_init();

    // libera interrupções
    sei();

    // primeira atualização de tela
    lcd_refresh();
    lcd_update = false;

    // loop principal – só faz a atualização de tela
    for(;;) {
        if (lcd_update) {
            lcd_refresh();
            lcd_update = false;
        }
    }
    return 0;
}