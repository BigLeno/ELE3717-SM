#include "snake.h"
#include "max7219.h"
#include "adc.h"
#include "lcd.h"
#include <stdlib.h>

void game_init(Game* game) {
    // Inicializar cobra sempre no centro (3,3) para garantir espaço
    game->snake.length = 3;
    game->snake.direction = DIR_RIGHT;
    game->snake.segments[0] = (Position){3, 3}; // cabeça no centro
    game->snake.segments[1] = (Position){2, 3}; // corpo
    game->snake.segments[2] = (Position){1, 3}; // cauda
    
    game->score = 0;
    game->game_over = 0;
    game->growth_timer = 0;
    game->move_speed_ms = INITIAL_MOVE_SPEED;
    game->game_over_timer = 0;
}

Direction get_joystick_direction(void) {
    static Direction current_dir = DIR_RIGHT;
    static Direction last_dir = DIR_RIGHT;
    
    uint16_t x_val = adc_read(4);
    uint16_t y_val = adc_read(5);
    
    // Deadzone otimizado para controle mais responsivo
    if (x_val < 300) {
        current_dir = DIR_LEFT;
    } else if (x_val > 724) {
        current_dir = DIR_RIGHT;
    } else if (y_val < 300) {
        current_dir = DIR_UP;
    } else if (y_val > 724) {
        current_dir = DIR_DOWN;
    }
    
    // Evitar que a cobra vá na direção oposta
    if ((current_dir == DIR_UP && last_dir == DIR_DOWN) ||
        (current_dir == DIR_DOWN && last_dir == DIR_UP) ||
        (current_dir == DIR_LEFT && last_dir == DIR_RIGHT) ||
        (current_dir == DIR_RIGHT && last_dir == DIR_LEFT)) {
        current_dir = last_dir;
    }
    
    last_dir = current_dir;
    return current_dir;
}

uint8_t check_collision(const Snake* snake) {
    Position head = snake->segments[0];
    
    // Colisão com paredes
    if (head.x < 0 || head.x >= BOARD_SIZE || head.y < 0 || head.y >= BOARD_SIZE) {
        return 1;
    }
    
    // Colisão com próprio corpo
    for (uint8_t i = 1; i < snake->length; i++) {
        if (head.x == snake->segments[i].x && head.y == snake->segments[i].y) {
            return 1;
        }
    }
    
    return 0;
}

void game_update(Game* game) {
    if (game->game_over) {
        // Incrementar timer de game over
        game->game_over_timer += game->move_speed_ms;
        return;
    }
    
    game->snake.direction = get_joystick_direction();
    
    // Incrementar timer de crescimento baseado no delay atual
    game->growth_timer += game->move_speed_ms;
    
    // Verificar se deve crescer
    if (game->growth_timer >= GROWTH_INTERVAL) {
        game->growth_timer = 0;
        
        // Crescer cobra
        if (game->snake.length < MAX_SNAKE_LENGTH) {
            game->snake.length++;
            game->score += 10;
            
            // Acelerar o movimento de forma mais suave
            if (game->move_speed_ms > MIN_MOVE_SPEED) {
                uint16_t speed_reduction = SPEED_DECREASE;
                
                // Aceleração progressiva mais equilibrada
                if (game->snake.length > 8) {
                    speed_reduction = SPEED_DECREASE + 3;
                }
                if (game->snake.length > 15) {
                    speed_reduction = SPEED_DECREASE + 5;
                }
                if (game->snake.length > 25) {
                    speed_reduction = SPEED_DECREASE + 8;
                }
                
                game->move_speed_ms -= speed_reduction;
                if (game->move_speed_ms < MIN_MOVE_SPEED) {
                    game->move_speed_ms = MIN_MOVE_SPEED;
                }
            }
        }
    }
    
    // Mover cobra
    Position new_head = game->snake.segments[0];
    
    switch (game->snake.direction) {
        case DIR_UP:    new_head.y = new_head.y - 1; break;
        case DIR_DOWN:  new_head.y = new_head.y + 1; break;
        case DIR_LEFT:  new_head.x = new_head.x - 1; break;
        case DIR_RIGHT: new_head.x = new_head.x + 1; break;
    }
    
    // Verificar colisões antes de mover
    if (new_head.x < 0 || new_head.x >= BOARD_SIZE || 
        new_head.y < 0 || new_head.y >= BOARD_SIZE) {
        game->game_over = 1;
        game->game_over_timer = 0;
        return;
    }
    
    // Verificar colisão com próprio corpo
    for (uint8_t i = 0; i < game->snake.length; i++) {
        if (new_head.x == game->snake.segments[i].x && 
            new_head.y == game->snake.segments[i].y) {
            game->game_over = 1;
            game->game_over_timer = 0;
            return;
        }
    }
    
    // Mover segmentos da cobra (shift para direita)
    for (uint8_t i = game->snake.length - 1; i > 0; i--) {
        game->snake.segments[i] = game->snake.segments[i - 1];
    }
    game->snake.segments[0] = new_head;
}

void draw_game(const Game* game) {
    max7219_clear();
    
    if (game->game_over) {
        // Animação de game over mais elaborada
        static uint8_t animation_phase = 0;
        animation_phase++;
        
        if ((animation_phase / 4) % 4 == 0) {
            // Fase 1: Toda a matriz acesa
            for (uint8_t i = 1; i <= 8; i++) {
                max7219_send(i, 0xFF);
            }
        } else if ((animation_phase / 4) % 4 == 1) {
            // Fase 2: Apenas bordas
            max7219_send(1, 0xFF); // Linha superior
            max7219_send(8, 0xFF); // Linha inferior
            for (uint8_t i = 2; i <= 7; i++) {
                max7219_send(i, 0x81); // Laterais
            }
        } else if ((animation_phase / 4) % 4 == 2) {
            // Fase 3: Cruz central
            for (uint8_t i = 1; i <= 8; i++) {
                if (i == 4 || i == 5) {
                    max7219_send(i, 0xFF); // Linhas centrais
                } else {
                    max7219_send(i, 0x18); // Colunas centrais
                }
            }
        } else {
            // Fase 4: Matriz vazia (piscar)
            for (uint8_t i = 1; i <= 8; i++) {
                max7219_send(i, 0x00);
            }
        }
        return;
    }
    
    uint8_t display_data[8] = {0};
    
    // Desenhar cobra com diferentes intensidades
    for (uint8_t i = 0; i < game->snake.length; i++) {
        uint8_t x = game->snake.segments[i].x;
        uint8_t y = game->snake.segments[i].y;
        if (x < 8 && y < 8) {
            display_data[y] |= (1 << x);
        }
    }
    
    // Destacar cabeça da cobra (piscar mais rápido)
    static uint8_t head_blink = 0;
    head_blink++;
    if ((head_blink / 3) % 2 && game->snake.length > 0) {
        uint8_t head_x = game->snake.segments[0].x;
        uint8_t head_y = game->snake.segments[0].y;
        if (head_x < 8 && head_y < 8) {
            display_data[head_y] ^= (1 << head_x); // XOR para piscar
        }
    }
    
    // Enviar dados para o display
    for (uint8_t i = 0; i < 8; i++) {
        max7219_send(i + 1, display_data[i]);
    }
}
