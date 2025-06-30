#include "snake.h"
#include "max7219.h"
#include "adc.h"
#include "lcd.h"
#include <stdlib.h>

void game_init(Game* game) {
    // Inicializar cobra sempre no centro (3,3) para garantir espaço
    game->snake.length = 3;
    game->snake.direction = DIR_RIGHT;
    game->snake.pending_direction = DIR_RIGHT; // Inicializar direção pendente
    game->snake.segments[0] = (Position){3, 3}; // cabeça no centro
    game->snake.segments[1] = (Position){2, 3}; // corpo
    game->snake.segments[2] = (Position){1, 3}; // cauda

    game->score = 0;
    game->game_over = 0;
    game->move_speed_ms = INITIAL_MOVE_SPEED;
    // Removido: game->growth_timer e game->game_over_timer
}

Direction get_joystick_direction(void) {
    uint16_t y_val = adc_read(4);
    uint16_t x_val = adc_read(5);

    // Ajuste fino: se necessário, altere o valor central para o seu joystick
    const int16_t x_center = 512;
    const int16_t y_center = 512;
    int16_t x_centered = (int16_t)x_val - x_center;
    int16_t y_centered = (int16_t)y_val - y_center;
    int16_t deadzone = 20; // zona morta ainda menor

    // Se ambos os eixos estão próximos do centro, não muda direção
    if (abs(x_centered) < deadzone && abs(y_centered) < deadzone) {
        return (Direction)255;
    }

    // Direção dominante: responde imediatamente ao movimento
    if (abs(y_centered) > abs(x_centered)) {
        if (y_centered > deadzone)  return DIR_UP;    // Para baixo (invertido)
        if (y_centered < -deadzone) return DIR_DOWN;      // Para cima (invertido)
    } else {
        if (x_centered < -deadzone) return DIR_RIGHT;
        if (x_centered > deadzone)  return DIR_LEFT;
    }

    return (Direction)255;
}

uint8_t check_collision(const Snake* snake) {
    Position head = snake->segments[0];

    // Colisão com paredes melhorada - verificação mais rigorosa
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
        // Não faz nada, animação controlada pelo main
        return;
    }

    // Ler direção do joystick a cada frame para máxima responsividade
    static Direction last_dir = DIR_RIGHT;
    Direction joystick_dir = get_joystick_direction();

    // Atualizar direção pendente com lógica melhorada e resposta mais fluida
    if (joystick_dir != (Direction)255 &&
        !((joystick_dir == DIR_UP && game->snake.direction == DIR_DOWN) ||
          (joystick_dir == DIR_DOWN && game->snake.direction == DIR_UP) ||
          (joystick_dir == DIR_LEFT && game->snake.direction == DIR_RIGHT) ||
          (joystick_dir == DIR_RIGHT && game->snake.direction == DIR_LEFT))) {
        last_dir = joystick_dir;
    }
    game->snake.direction = last_dir;

    // Mover cobra
    Position new_head = game->snake.segments[0];

    switch (game->snake.direction) {
        case DIR_UP:    new_head.y = new_head.y - 1; break;
        case DIR_DOWN:  new_head.y = new_head.y + 1; break;
        case DIR_LEFT:  new_head.x = new_head.x - 1; break;
        case DIR_RIGHT: new_head.x = new_head.x + 1; break;
    }

    // Movimento com wrap-around (teletransporte nas bordas)
    if (new_head.x < 0) new_head.x = 7;
    if (new_head.x > 7) new_head.x = 0;
    if (new_head.y < 0) new_head.y = 7;
    if (new_head.y > 7) new_head.y = 0;

    // Mover segmentos da cobra
    for (uint8_t i = game->snake.length - 1; i > 0; i--) {
        game->snake.segments[i] = game->snake.segments[i - 1];
    }
    game->snake.segments[0] = new_head;
}

void draw_game(const Game* game) {
    max7219_clear();

    /*
    // MODO DE CALIBRAÇÃO DO JOYSTICK (descomente para testar)
    Direction dir = get_joystick_direction();
    uint8_t pattern[8] = {0};
    if (dir == DIR_UP) {
        pattern[0] = 0xFF;
    } else if (dir == DIR_DOWN) {
        pattern[7] = 0xFF;
    } else if (dir == DIR_LEFT) {
        for (uint8_t i = 0; i < 8; i++) pattern[i] = 0x80;
    } else if (dir == DIR_RIGHT) {
        for (uint8_t i = 0; i < 8; i++) pattern[i] = 0x01;
    } else {
        pattern[3] = 0x18;
        pattern[4] = 0x18;
    }
    for (uint8_t i = 0; i < 8; i++) {
        max7219_send(i + 1, pattern[i]);
    }
    return;
    */

    // --- JOGO SNAKE NORMAL ---
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
    
    // Desenhar cobra
    for (uint8_t i = 0; i < game->snake.length; i++) {
        uint8_t x = game->snake.segments[i].x;
        uint8_t y = game->snake.segments[i].y;
        if (x < 8 && y < 8) {
            display_data[y] |= (1 << x);
        }
    }
    // Enviar dados para o display
    for (uint8_t i = 0; i < 8; i++) {
        max7219_send(i + 1, display_data[i]);
    }
}
