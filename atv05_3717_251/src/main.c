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
    
    
    Game game;
    game_init(&game);
    uint8_t last_length = 3;
    
    while (1) {
        game_update(&game);
        draw_game(&game);

        // Atualizar LCD sempre mostrando as posições X e Y da cobra
        lcd_clear();
        lcd_goto(0, 0);
        lcd_print("X:");
        for (uint8_t i = 0; i < MAX_SNAKE_LENGTH; i++) {
            lcd_print(" ");
            if (i < game.snake.length) {
                lcd_print_dec(game.snake.segments[i].x);
            } else {
                lcd_print("0");
            }
        }
        lcd_goto(1, 0);
        lcd_print("Y:");
        for (uint8_t i = 0; i < MAX_SNAKE_LENGTH; i++) {
            lcd_print(" ");
            if (i < game.snake.length) {
                lcd_print_dec(game.snake.segments[i].y);
            } else {
                lcd_print("0");
            }
        }

        // Reset automático do jogo após animação de game over
        if (game.game_over && game.game_over_timer >= GAME_OVER_ANIMATION_TIME) {
            game_init(&game); // Reiniciar jogo automaticamente
            last_length = 3;
        }
        
        // Velocidade otimizada para máxima jogabilidade
        uint16_t current_delay;
        
        if (game.game_over) {
            // Durante game over, usar delay rápido para animação fluida
            current_delay = 60; // Aumenta o delay da animação de game over
        } else {
            // Durante jogo, usar velocidade mais confortável
            current_delay = game.move_speed_ms;
            if (current_delay < 120) current_delay = 120;   // Mínimo mais lento
            if (current_delay > 400) current_delay = 400;   // Máximo mais lento
        }
        
        variable_delay_ms(current_delay);
    }
    
    return 0;
}