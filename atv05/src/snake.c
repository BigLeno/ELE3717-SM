#include "snake.h"
#include "max7219.h"
#include "adc.h"
#include "lcd.h"
#include <stdlib.h>

void game_init(Game* game) {
    // Inicializar cobra no centro
    game->snake.length = 3;
    game->snake.direction = DIR_RIGHT;
    game->snake.segments[0] = (Position){4, 4}; // cabeça
    game->snake.segments[1] = (Position){3, 4};
    game->snake.segments[2] = (Position){2, 4}; // cauda
    
    game->score = 0;
    game->game_over = 0;
    
    spawn_food(game);
}

Direction get_joystick_direction(void) {
    static Direction current_dir = DIR_RIGHT;
    static Direction last_dir = DIR_RIGHT;
    
    uint16_t x_val = adc_read(4);
    uint16_t y_val = adc_read(5);
    
    // Zona morta para evitar mudanças acidentais
    if (x_val < 300) {
        current_dir = DIR_LEFT;
    } else if (x_val > 700) {
        current_dir = DIR_RIGHT;
    } else if (y_val < 300) {
        current_dir = DIR_UP;
    } else if (y_val > 700) {
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

void spawn_food(Game* game) {
    uint8_t valid_position = 0;
    
    while (!valid_position) {
        game->food.x = rand() % BOARD_SIZE;
        game->food.y = rand() % BOARD_SIZE;
        
        valid_position = 1;
        // Verificar se a comida não está na cobra
        for (uint8_t i = 0; i < game->snake.length; i++) {
            if (game->snake.segments[i].x == game->food.x && 
                game->snake.segments[i].y == game->food.y) {
                valid_position = 0;
                break;
            }
        }
    }
}

uint8_t check_collision(const Snake* snake) {
    Position head = snake->segments[0];
    
    // Colisão com paredes
    if (head.x >= BOARD_SIZE || head.y >= BOARD_SIZE) {
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
    if (game->game_over) return;
    
    game->snake.direction = get_joystick_direction();
    
    // Mover cobra
    Position new_head = game->snake.segments[0];
    
    switch (game->snake.direction) {
        case DIR_UP:    new_head.y = (new_head.y == 0) ? BOARD_SIZE - 1 : new_head.y - 1; break;
        case DIR_DOWN:  new_head.y = (new_head.y + 1) % BOARD_SIZE; break;
        case DIR_LEFT:  new_head.x = (new_head.x == 0) ? BOARD_SIZE - 1 : new_head.x - 1; break;
        case DIR_RIGHT: new_head.x = (new_head.x + 1) % BOARD_SIZE; break;
    }
    
    // Verificar se comeu a comida
    uint8_t ate_food = (new_head.x == game->food.x && new_head.y == game->food.y);
    
    // Mover segmentos
    for (uint8_t i = game->snake.length; i > 0; i--) {
        game->snake.segments[i] = game->snake.segments[i - 1];
    }
    game->snake.segments[0] = new_head;
    
    if (ate_food) {
        game->snake.length++;
        game->score += 10;
        spawn_food(game);
    }
    
    // Verificar colisões
    if (check_collision(&game->snake)) {
        game->game_over = 1;
    }
}

void draw_game(const Game* game) {
    max7219_clear();
    
    if (game->game_over) {
        // Mostrar padrão de game over
        for (uint8_t i = 1; i <= 8; i++) {
            max7219_send(i, 0xFF);
        }
        return;
    }
    
    uint8_t display_data[8] = {0};
    
    // Desenhar cobra
    for (uint8_t i = 0; i < game->snake.length; i++) {
        uint8_t x = game->snake.segments[i].x;
        uint8_t y = game->snake.segments[i].y;
        if (x < 8 && y < 8) {
            display_data[y] |= (1 << x);
        }
    }
    
    // Desenhar comida (piscar)
    static uint8_t blink_counter = 0;
    blink_counter++;
    if ((blink_counter / 5) % 2) { // Piscar a cada ~5 frames
        if (game->food.x < 8 && game->food.y < 8) {
            display_data[game->food.y] |= (1 << game->food.x);
        }
    }
    
    // Enviar dados para o display
    for (uint8_t i = 0; i < 8; i++) {
        max7219_send(i + 1, display_data[i]);
    }
}
