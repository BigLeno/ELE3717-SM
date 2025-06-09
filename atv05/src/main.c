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
    lcd_print("Score: 0");
    
    uint16_t last_score = 0;
    
    while (1) {
        game_update(&game);
        draw_game(&game);
        
        // Atualizar LCD apenas quando necessário
        if (game.score != last_score || game.game_over) {
            lcd_clear();
            if (game.game_over) {
                lcd_goto(0, 0);
                lcd_print("GAME OVER!");
                lcd_goto(1, 0);
                lcd_print("Score: ");
                lcd_print_dec(game.score);
            } else {
                lcd_goto(0, 0);
                lcd_print("Snake Game!");
                lcd_goto(1, 0);
                lcd_print("Score: ");
                lcd_print_dec(game.score);
            }
            last_score = game.score;
        }
        
        // Reset do jogo se estiver em game over
        if (game.game_over) {
            _delay_ms(3000); // Esperar 3 segundos
            game_init(&game); // Reiniciar jogo
            last_score = 0;
        }
        
        _delay_ms(200); // Velocidade do jogo
    }
    
    return 0;
}