#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>

#include "lcd.h"
#include "spi.h"
#include "adc.h"
#include "max7219.h"
#include "snake.h"

// Função para delay variável
void variable_delay_ms(uint16_t ms) {
    while (ms > 0) {
        if (ms >= 10) {
            _delay_ms(10);
            ms -= 10;
        } else {
            _delay_ms(1);
            ms--;
        }
    }
}

int main() {
    // Inicializar periféricos
    spi_init();
    max7219_init();
    max7219_clear();
    adc_init();
    lcd_init();
    
    // Inicializar gerador de números aleatórios
    srand(adc_read(0)); // Usar ruído do ADC como semente
    
    Game game;
    game_init(&game);
    
    lcd_clear();
    lcd_goto(0, 0);
    lcd_print("Snake Game!");
    lcd_goto(1, 0);
    lcd_print("Length: 3");
    
    uint8_t last_length = 3;
    uint16_t last_speed = INITIAL_MOVE_SPEED;
    
    while (1) {
        game_update(&game);
        draw_game(&game);
        
        // Atualizar LCD quando necessário
        if (game.snake.length != last_length || game.move_speed_ms != last_speed || game.game_over) {
            lcd_clear();
            if (game.game_over) {
                lcd_goto(0, 0);
                lcd_print("GAME OVER!");
                lcd_goto(1, 0);
                lcd_print("Final: ");
                lcd_print_dec(game.snake.length);
            } else {
                lcd_goto(0, 0);
                lcd_print("Snake Game!");
                lcd_goto(1, 0);
                lcd_print("Length: ");
                lcd_print_dec(game.snake.length);
            }
            last_length = game.snake.length;
            last_speed = game.move_speed_ms;
        }
        
        // Reset automático do jogo após animação de game over
        if (game.game_over && game.game_over_timer >= GAME_OVER_ANIMATION_TIME) {
            game_init(&game); // Reiniciar jogo automaticamente
            last_length = 3;
            last_speed = INITIAL_MOVE_SPEED;
            
            // Mostrar tela de reinício brevemente
            lcd_clear();
            lcd_goto(0, 0);
            lcd_print("Restarting...");
            _delay_ms(500);
        }
        
        // Usar velocidade de movimento otimizada
        uint16_t current_delay = game.move_speed_ms;
        if (current_delay < 30) current_delay = 30;   // Mínimo mais responsivo
        if (current_delay > 250) current_delay = 250; // Máximo mais ágil
        
        variable_delay_ms(current_delay);
    }
    
    return 0;
}