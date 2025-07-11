#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"
#include "mpu6050.h"
#include "USART.h"

void setup(void);
static void vtask_led(void *pvParameters);
static void vtask_mpu6050(void *pvParameters);

int main(void)
{
	setup();

	xTaskCreate(vtask_led, (const char *)"led", 256, NULL, 1, NULL);
	xTaskCreate(vtask_mpu6050, (const char *)"mpu6050", 512, NULL, 2, NULL);
	vTaskStartScheduler();
	for (;;);
}

void setup() {
	DDRB |= (1 << PB5);  // Configura PB5 (LED) como saída
	PORTB = 0x00;
	
	// Inicializa a USART
	USART_init(MYUBRR);
	
	// Inicializa o MPU6050
	mpu6050_init();
	
	// Mensagem de inicialização
	USART_send_string("Sistema iniciado!\r\n");
	USART_send_string("Formato: AX,AY,AZ,GX,GY,GZ,TEMP\r\n");
}

static void vtask_led(void *pvParameters)
{
	for (;;)
	{
		PORTB ^= (1 << PB5); 
		vTaskDelay(pdMS_TO_TICKS(500));  // Delay de 500ms
	}
}

static void vtask_mpu6050(void *pvParameters)
{
	mpu6050_data_t mpu_data;
	
	// Verifica se o MPU6050 está conectado
	if (!mpu6050_test_connection()) {
		// Se não conseguir conectar, envia erro pela serial
		USART_send_string("ERRO: MPU6050 nao encontrado!\r\n");
		// E pisca o LED rapidamente
		for (;;) {
			PORTB ^= (1 << PB5);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
	
	USART_send_string("MPU6050 conectado com sucesso!\r\n");
	
	for (;;)
	{
		// Lê todos os dados do MPU6050
		mpu6050_read_all(&mpu_data);
		
		// Envia dados do acelerômetro
		USART_send_int(mpu_data.accel_x);
		USART_send_string(",");
		USART_send_int(mpu_data.accel_y);
		USART_send_string(",");
		USART_send_int(mpu_data.accel_z);
		USART_send_string(",");
		
		// Envia dados do giroscópio
		USART_send_int(mpu_data.gyro_x);
		USART_send_string(",");
		USART_send_int(mpu_data.gyro_y);
		USART_send_string(",");
		USART_send_int(mpu_data.gyro_z);
		USART_send_string(",");
		
		// Envia temperatura
		USART_send_int(mpu_data.temp);
		USART_send_string("\r\n");
		
		// Delay de 250ms entre leituras (4Hz para não saturar a serial)
		vTaskDelay(pdMS_TO_TICKS(250));
	}
}