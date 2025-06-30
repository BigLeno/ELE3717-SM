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

    // Timers para cada ação
    uint16_t move_timer = 0;
    uint16_t growth_timer = 0;
    uint16_t game_over_timer = 0;
    uint16_t lcd_timer = 0;

    // Intervalos em ms
    const uint16_t loop_delay = 20; // Loop principal roda a cada 20ms
    const uint16_t lcd_interval = 120; // Atualiza LCD a cada 120ms

    while (1) {
        // Timers incrementam a cada iteração
        move_timer += loop_delay;
        growth_timer += loop_delay;
        lcd_timer += loop_delay;
        if (game.game_over)
            game_over_timer += loop_delay;

        // Movimento da cobra
        if (!game.game_over && move_timer >= game.move_speed_ms) {
            move_timer = 0;
            game_update(&game); // Só move a cobra aqui
        }

        // Crescimento da cobra
        if (!game.game_over && growth_timer >= GROWTH_INTERVAL) {
            growth_timer = 0;
            // Cresce a cobra e ajusta velocidade/score
            if (game.snake.length < MAX_SNAKE_LENGTH) {
                game.snake.length++;
                game.score += 10;
                if (game.move_speed_ms > MIN_MOVE_SPEED) {
                    uint16_t speed_reduction = SPEED_DECREASE;
                    if (game.snake.length > 8) speed_reduction = SPEED_DECREASE + 1;
                    if (game.snake.length > 15) speed_reduction = SPEED_DECREASE + 2;
                    if (game.snake.length > 25) speed_reduction = SPEED_DECREASE + 3;
                    if (game.snake.length > 35) speed_reduction = SPEED_DECREASE + 4;
                    game.move_speed_ms -= speed_reduction;
                    if (game.move_speed_ms < MIN_MOVE_SPEED)
                        game.move_speed_ms = MIN_MOVE_SPEED;
                }
            }
        }

        // Animação de game over
        if (game.game_over && game_over_timer >= GAME_OVER_ANIMATION_TIME) {
            game_init(&game);
            move_timer = 0;
            growth_timer = 0;
            game_over_timer = 0;
            lcd_timer = 0;
        }

        // Atualização do LCD
        if (lcd_timer >= lcd_interval) {
            lcd_timer = 0;
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
        }

        // Desenhar sempre (animação do display)
        draw_game(&game);

        _delay_ms(loop_delay);
    }

    return 0;
}