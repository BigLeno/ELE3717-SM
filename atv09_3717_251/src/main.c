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
	
	// Mensagem de inicialização melhorada
	USART_send_string("\r\n");
	USART_send_string("========================================\r\n");
	USART_send_string("    SISTEMA MPU6050 + FreeRTOS\r\n");
	USART_send_string("========================================\r\n");
	USART_send_string("Inicializando...\r\n");
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
	uint32_t sample_count = 0;
	
	// Verifica se o MPU6050 está conectado
	if (!mpu6050_test_connection()) {
		// Se não conseguir conectar, envia erro pela serial
		USART_send_string("ERRO: MPU6050 nao encontrado!\r\n");
		USART_send_string("Verifique as conexoes I2C (SDA/SCL)\r\n");
		USART_send_string("LED piscando rapidamente...\r\n");
		// E pisca o LED rapidamente
		for (;;) {
			PORTB ^= (1 << PB5);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
	
	USART_send_string("MPU6050 conectado com sucesso!\r\n");
	USART_send_string("Configuracoes:\r\n");
	USART_send_string("- Acelerometro: +/- 2g\r\n");
	USART_send_string("- Giroscopio: +/- 250 graus/s\r\n");
	USART_send_string("- Taxa de amostragem: 4Hz\r\n");
	USART_send_string("========================================\r\n");
	USART_send_string("Iniciando leituras...\r\n\r\n");
	
	for (;;)
	{
		// Lê todos os dados do MPU6050
		mpu6050_read_all(&mpu_data);
		sample_count++;
		
		// Cabeçalho da amostra
		USART_send_string("--- Amostra #");
		USART_send_int(sample_count);
		USART_send_string(" ---\r\n");
		
		// Dados do acelerômetro com rótulos
		USART_send_string("Acelerometro (raw):\r\n");
		USART_send_string("  X: ");
		USART_send_int(mpu_data.accel_x);
		USART_send_string("  Y: ");
		USART_send_int(mpu_data.accel_y);
		USART_send_string("  Z: ");
		USART_send_int(mpu_data.accel_z);
		USART_send_string("\r\n");
		
		// Dados do giroscópio com rótulos
		USART_send_string("Giroscopio (raw):\r\n");
		USART_send_string("  X: ");
		USART_send_int(mpu_data.gyro_x);
		USART_send_string("  Y: ");
		USART_send_int(mpu_data.gyro_y);
		USART_send_string("  Z: ");
		USART_send_int(mpu_data.gyro_z);
		USART_send_string("\r\n");
		
		// Temperatura
		USART_send_string("Temperatura (raw): ");
		USART_send_int(mpu_data.temp);
		USART_send_string("\r\n");
		
		// Separador
		USART_send_string("\r\n");
		
		// Delay de 1 segundo entre leituras para melhor visualização
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}