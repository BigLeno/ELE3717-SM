#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdint.h>

// Defini��es para LCD 4 bits em PORTD
#define PORT_LCD PORTD
#define DDR_LCD  DDRD
#define RS PD2
#define EN PD3
#define LCD_DATA_MASK ((1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7))

// Bot�es nos pinos PC1, PC2, PC3
#define BTN_S1 PC1 // Sele��o e ciclo do asterisco
#define BTN_S2 PC2 // Incremento
#define BTN_S3 PC3 // Decremento

#define set_bit(Y, bit_x)  ((Y) |= (1 << (bit_x)))
#define clr_bit(Y, bit_x)  ((Y) &= ~(1 << (bit_x)))

// Delays e per�odos para auto-repeat
#define BTN_DEBOUNCE_MS    20
#define AUTO_REPEAT_WAIT   5000  // 5 segundos para ativar auto-repeat
#define AUTO_REPEAT_FAST   100   // 100ms entre incrementos r�pidos
#define AUTO_REPEAT_SLOW   500   // 500ms entre incrementos lentos

// Fun��es LCD (como antes)
void lcd_send_nibble(uint8_t val)
{
	PORT_LCD &= ~LCD_DATA_MASK;
	PORT_LCD |= ((val << 4) & LCD_DATA_MASK);
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

	if (rs == 0 && (c == 0x01 || c == 0x02))
	_delay_ms(2);
	else
	_delay_us(40);
}

void lcd_init(void)
{
	DDR_LCD |= LCD_DATA_MASK | (1 << RS) | (1 << EN);
	_delay_ms(40);
	clr_bit(PORT_LCD, RS);

	lcd_send_nibble(0x03);
	_delay_ms(5);
	lcd_send_nibble(0x03);
	_delay_us(150);
	lcd_send_nibble(0x03);
	_delay_us(150);
	lcd_send_nibble(0x02);
	_delay_us(150);

	lcd_send_byte(0x28, 0);
	lcd_send_byte(0x08, 0);
	lcd_send_byte(0x01, 0);
	_delay_ms(2);
	lcd_send_byte(0x06, 0);
	lcd_send_byte(0x0C, 0);
}

void lcd_clear(void)
{
	lcd_send_byte(0x01, 0);
	_delay_ms(2);
}

void lcd_goto(uint8_t linha, uint8_t coluna)
{
	uint8_t addr = coluna + (linha ? 0x40 : 0x00);
	lcd_send_byte(0x80 | addr, 0);
}

void lcd_print(char *str)
{
	while (*str)
	lcd_send_byte(*str++, 1);
}

// Imprime valor decimal (3 d�gitos) sem usar sprintf
void lcd_print_num(uint8_t val)
{
	char buf[3];
	buf[0] = (val / 100) + '0';
	buf[1] = ((val / 10) % 10) + '0';
	buf[2] = (val % 10) + '0';

	for (int i = 0; i < 3; i++)
	{
		lcd_send_byte(buf[i], 1);
	}
}

// Bot�es ------------------------------------------------------------
// Setup dos bot�es com pull-ups
void buttons_init()
{
	DDRC &= ~((1 << BTN_S1) | (1 << BTN_S2) | (1 << BTN_S3));
	PORTC |= (1 << BTN_S1) | (1 << BTN_S2) | (1 << BTN_S3);
}

// Leitura simples com debounce b�sica (20ms)
bool button_pressed(uint8_t pin)
{
	if ((PINC & (1 << pin)) == 0)
	{
		_delay_ms(BTN_DEBOUNCE_MS);
		if ((PINC & (1 << pin)) == 0)
		return true;
	}
	return false;
}

// Vari�veis para controlar auto-repeat dos bot�es S2 e S3
typedef struct {
	bool pressed;
	uint32_t press_start_ms;
	uint32_t last_act_ms;
	bool auto_repeat_active;
} btn_state_t;

// Vari�veis globais para auto-repeat
volatile btn_state_t btn_inc = {0};
volatile btn_state_t btn_dec = {0};

// Temporizador simples em millis (usa delay_ms acumulativo)
volatile uint32_t millis_count = 0;

// Fun��o para delay n�o bloqueante - atualiza contador global
void delay_ms_tick(void)
{
	_delay_ms(1);
	millis_count++;
}

// Atualiza estado da press�o de bot�es com debounce e controle de auto-repeat
void btn_update(volatile btn_state_t *btn, uint8_t pin)
{
	bool currently_pressed = ((PINC & (1 << pin)) == 0);

	if (currently_pressed && !btn->pressed)
	{
		btn->pressed = true;
		btn->press_start_ms = millis_count;
		btn->last_act_ms = millis_count;
		btn->auto_repeat_active = false;
	}
	else if (!currently_pressed && btn->pressed)
	{
		btn->pressed = false;
		btn->auto_repeat_active = false;
	}
	else if (btn->pressed)
	{
		uint32_t held_time = millis_count - btn->press_start_ms;

		if (held_time >= AUTO_REPEAT_WAIT)
		{
			uint32_t since_last = millis_count - btn->last_act_ms;
			uint32_t interval = (held_time < AUTO_REPEAT_WAIT * 2) ? AUTO_REPEAT_SLOW : AUTO_REPEAT_FAST;
			if (since_last >= interval)
			{
				btn->auto_repeat_active = true;
				btn->last_act_ms = millis_count;
			}
		}
	}
}

// Checa se o bot�o S2/S3 teve evento para incrementar ou decrementar
// Retorna true se deve aumentar/diminuir valor.
bool btn_should_act(volatile btn_state_t *btn, uint8_t pin)
{
	static bool last_state_inc = false;
	static bool last_state_dec = false;

	bool current = ((PINC & (1 << pin)) == 0);

	bool triggered = false;

	if (pin == BTN_S2)
	{
		if (current && !last_state_inc) triggered = true;
		last_state_inc = current;
	}
	else if (pin == BTN_S3)
	{
		if (current && !last_state_dec) triggered = true;
		last_state_dec = current;
	}
	return triggered || btn->auto_repeat_active;
}

// Vari�veis de controle
volatile uint8_t red_val = 0;
volatile uint8_t green_val = 0;
volatile uint8_t blue_val = 0;

volatile int8_t seletor = 0; // -1 = sem sele��o, 0=RED, 1=GREEN, 2=BLUE

// Atualiza LCD
void lcd_atualiza_display(void)
{
	lcd_clear();
	lcd_goto(0, 0);
	lcd_print("RED  GREEN BLUE");

	lcd_goto(1, 0);

	// RED
	lcd_print_num(red_val);
	if (seletor == 0)
	lcd_send_byte('*', 1);
	else
	lcd_send_byte(' ', 1);
	lcd_send_byte(' ', 1);

	// GREEN
	lcd_print_num(green_val);
	if (seletor == 1)
	lcd_send_byte('*', 1);
	else
	lcd_send_byte(' ', 1);
	lcd_send_byte(' ', 1);

	// BLUE
	lcd_print_num(blue_val);
	if (seletor == 2)
	lcd_send_byte('*', 1);
	else
	lcd_send_byte(' ', 1);
}

// Programa principal
int main(void)
{
	buttons_init();
	lcd_init();
	lcd_atualiza_display();

	btn_inc.pressed = false;
	btn_dec.pressed = false;
	btn_inc.auto_repeat_active = false;
	btn_dec.auto_repeat_active = false;

	while (1)
	{
		delay_ms_tick();

		btn_update(&btn_inc, BTN_S2);
		btn_update(&btn_dec, BTN_S3);

		// Bot�o S1: ciclo seletor e toggle asterisco
		if (button_pressed(BTN_S1))
		{
			_delay_ms(100); // debounce seguro

			seletor++;
			if (seletor > 3)
			seletor = 0;

			if (seletor == 3)
			seletor = -1; // sem sele��o, asterisco desaparece

			lcd_atualiza_display();

			while (button_pressed(BTN_S1))
			delay_ms_tick();
		}

		// Incremento
		if (seletor != -1 && btn_should_act(&btn_inc, BTN_S2))
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
		}

		// Decremento
		if (seletor != -1 && btn_should_act(&btn_dec, BTN_S3))
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
		}
	}
}