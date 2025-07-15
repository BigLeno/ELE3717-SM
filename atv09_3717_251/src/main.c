#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>  // Para abs()

#include "FreeRTOS.h"
#include "task.h"
#include "mpu6050.h"
#include "USART.h"

// ATENÇÃO: O pino TX (PD0) está conectado ao módulo Bluetooth HC-05
// Todos os dados enviados via USART serão transmitidos pelo Bluetooth
// Não é necessário modificar o código de transmissão

// Tabela de lookup para arctangente (atan) 
// Índices: ratio * 10 (0 a 50, que corresponde a 0.0 a 5.0)
// Valores: ângulo em graus * 10 (para preservar uma casa decimal)
const int16_t atan_lookup[51] = {
	0,    57,   114,  171,  228,  284,  340,  395,  449,  503,  // 0.0-0.9
	557,  610,  662,  714,  765,  816,  866,  916,  965,  1013, // 1.0-1.9
	1061, 1107, 1154, 1199, 1244, 1288, 1332, 1376, 1419, 1462, // 2.0-2.9
	1504, 1546, 1588, 1629, 1670, 1711, 1751, 1791, 1831, 1871, // 3.0-3.9
	1910, 1949, 1988, 2027, 2065, 2103, 2141, 2179, 2217, 2254, // 4.0-4.9
	2291  // 5.0
};

// Função para calcular arctangente usando lookup table
int16_t fast_atan2_degrees(int16_t y, int16_t z) {
	if (z == 0) return (y > 0) ? 900 : -900; // ±90°
	
	// Calcula a razão absoluta
	int32_t ratio_abs = (y >= 0 ? y : -y) * 10L;
	int32_t z_abs = (z >= 0 ? z : -z);
	ratio_abs = ratio_abs / z_abs;
	
	// Limita o índice da tabela
	if (ratio_abs > 50) ratio_abs = 50;
	
	int16_t angle = atan_lookup[ratio_abs];
	
	// Ajusta o sinal baseado nos quadrantes
	if (z < 0) angle = 1800 - angle; // 180° - angle
	if (y < 0) angle = -angle;
	
	return angle; // Retorna em décimos de grau
}

void setup(void);
static void vtask_mpu6050(void *pvParameters);

int main(void)
{
	setup();

	USART_send_string("DEBUG: Criando task MPU6050...\r\n");
	
	// Apenas uma task para economizar RAM
	xTaskCreate(vtask_mpu6050, (const char *)"mpu6050", 192, NULL, 1, NULL);
	USART_send_string("DEBUG: Task MPU6050 criada!\r\n");
	
	USART_send_string("DEBUG: Iniciando scheduler...\r\n");
	vTaskStartScheduler();
	
	// Se chegou aqui, o scheduler falhou
	USART_send_string("ERRO: Scheduler falhou!\r\n");
	for (;;);
}

void setup() {
	DDRB |= (1 << PB5);  // Configura PB5 (LED) como saída
	PORTB = 0x00;
	
	// Inicializa a USART
	USART_init(MYUBRR);
	
	// Mensagem de inicialização melhorada
	USART_send_string("\r\n");
	USART_send_string("========================================\r\n");
	USART_send_string("    SISTEMA MPU6050 + FreeRTOS\r\n");
	USART_send_string("========================================\r\n");
	USART_send_string("Inicializando...\r\n");
	
	// Debug: Antes da inicialização do MPU6050
	USART_send_string("DEBUG: Iniciando MPU6050...\r\n");
	
	// Inicializa o MPU6050
	mpu6050_init();
	
	// Debug: Após inicialização
	USART_send_string("DEBUG: MPU6050 inicializado!\r\n");
}

static void vtask_mpu6050(void *pvParameters)
{
	mpu6050_data_t mpu_data;
	uint32_t sample_count = 0;
	uint8_t led_state = 0;
	
	// Debug: Task iniciada
	USART_send_string("DEBUG: Task MPU6050 iniciada!\r\n");
	
	// Verifica se o MPU6050 está conectado
	USART_send_string("DEBUG: Testando conexao MPU6050...\r\n");
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
		// Controle do LED (pisca a cada leitura)
		led_state = !led_state;
		if (led_state) {
			PORTB |= (1 << PB5);  // Liga LED
		} else {
			PORTB &= ~(1 << PB5); // Desliga LED
		}
		// Lê todos os dados do MPU6050
		mpu6050_read_all(&mpu_data);
		sample_count++;
		
		// Calcula ângulos usando tabela de lookup (sem math.h)
		int16_t roll_tenths = fast_atan2_degrees(mpu_data.accel_y, mpu_data.accel_z);
		int16_t pitch_tenths = fast_atan2_degrees(-mpu_data.accel_x, mpu_data.accel_z);
		
		// Cabeçalho da amostra
		USART_send_string("--- Amostra #");
		USART_send_int(sample_count);
		USART_send_string(" ---\r\n");
		
		// Dados do acelerômetro
		USART_send_string("Acelerometro:\r\n");
		USART_send_string("  X: ");
		USART_send_int(mpu_data.accel_x);
		USART_send_string(" raw (");
		USART_send_int(mpu_data.accel_x / 16384); // Conversão aproximada para g
		USART_send_string("g)\r\n");
		
		USART_send_string("  Y: ");
		USART_send_int(mpu_data.accel_y);
		USART_send_string(" raw (");
		USART_send_int(mpu_data.accel_y / 16384);
		USART_send_string("g)\r\n");
		
		USART_send_string("  Z: ");
		USART_send_int(mpu_data.accel_z);
		USART_send_string(" raw (");
		USART_send_int(mpu_data.accel_z / 16384);
		USART_send_string("g)\r\n");
		
		// Dados do giroscópio
		USART_send_string("Giroscopio:\r\n");
		USART_send_string("  X: ");
		USART_send_int(mpu_data.gyro_x);
		USART_send_string(" raw (");
		USART_send_int(mpu_data.gyro_x / 131); // Conversão aproximada para °/s
		USART_send_string(" deg/s)\r\n");
		
		USART_send_string("  Y: ");
		USART_send_int(mpu_data.gyro_y);
		USART_send_string(" raw (");
		USART_send_int(mpu_data.gyro_y / 131);
		USART_send_string(" deg/s)\r\n");
		
		USART_send_string("  Z: ");
		USART_send_int(mpu_data.gyro_z);
		USART_send_string(" raw (");
		USART_send_int(mpu_data.gyro_z / 131);
		USART_send_string(" deg/s)\r\n");
		
		// Ângulos calculados usando lookup table (sem math.h)
		USART_send_string("Orientacao (lookup table):\r\n");
		USART_send_string("  Roll:  ");
		USART_send_int(roll_tenths / 10);  // Parte inteira
		USART_send_string(".");
		USART_send_int(abs(roll_tenths % 10)); // Uma casa decimal
		USART_send_string("°\r\n");
		
		USART_send_string("  Pitch: ");
		USART_send_int(pitch_tenths / 10);  // Parte inteira
		USART_send_string(".");
		USART_send_int(abs(pitch_tenths % 10)); // Uma casa decimal
		USART_send_string("°\r\n");
		
		// Temperatura com conversão
		USART_send_string("Temperatura: ");
		USART_send_int(mpu_data.temp);
		USART_send_string(" raw (");
		USART_send_int((mpu_data.temp / 340) + 37); // Conversão aproximada para °C
		USART_send_string(" °C)\r\n");
		
		// Separador
		USART_send_string("\r\n");
		
		// Delay de 1 segundo entre leituras para melhor visualização
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}