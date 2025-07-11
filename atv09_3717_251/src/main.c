#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"

void setup(void);
static void vtask_led(void *pvParameters);

int main(void)
{
	setup();

	xTaskCreate(vtask_led, (const char *)"led", 256, NULL, 1, NULL);
	vTaskStartScheduler();
	for (;;);
}

void setup() {
	DDRB |= (1 << PB5);  // Configura PB5 (LED) como saída
	PORTB = 0x00;
}

static void vtask_led(void *pvParameters)
{
	for (;;)
	{
		PORTB ^= (1 << PB5); 
		vTaskDelay(pdMS_TO_TICKS(500));  // Delay de 500ms
	}
}