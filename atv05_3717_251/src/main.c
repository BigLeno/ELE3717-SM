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

// Função para delay variável usando loops com constantes
void variable_delay_ms(uint16_t ms) {
    while (ms >= 100) {
        _delay_ms(100);
        ms -= 100;
    }
    while (ms >= 10) {
        _delay_ms(10);
        ms -= 10;
    }
    while (ms >= 1) {
        _delay_ms(1);
        ms -= 1;
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
    
    while (1) {
        game_update(&game);
        draw_game(&game);
        
        // Atualizar LCD apenas quando realmente necessário
        if (game.snake.length != last_length || game.game_over) {
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
        }
        
        // Reset automático do jogo após animação de game over
        if (game.game_over && game.game_over_timer >= GAME_OVER_ANIMATION_TIME) {
            game_init(&game); // Reiniciar jogo automaticamente
            last_length = 3;
            
            // Mostrar tela de reinício rapidamente
            lcd_clear();
            lcd_goto(0, 0);
            lcd_print("New Game!");
            _delay_ms(150); // Delay ainda mais reduzido
        }
        
        // Velocidade otimizada para máxima jogabilidade
        uint16_t current_delay;
        
        if (game.game_over) {
            // Durante game over, usar delay rápido para animação fluida
            current_delay = 30;
        } else {
            // Durante jogo, usar velocidade extremamente responsiva
            current_delay = game.move_speed_ms;
            if (current_delay < 40) current_delay = 40;   // Mínimo ultra responsivo
            if (current_delay > 280) current_delay = 280; // Máximo mais ágil
        }
        
        variable_delay_ms(current_delay);
    }
    
    return 0;
}