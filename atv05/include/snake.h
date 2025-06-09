#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

#define BOARD_SIZE 8
#define MAX_SNAKE_LENGTH 64
#define INITIAL_MOVE_SPEED 200   // Movimento mais fluido (200ms)
#define SPEED_DECREASE 10        // Aceleração mais suave (10ms)
#define MIN_MOVE_SPEED 60        // Velocidade mínima mais rápida (60ms)
#define GROWTH_INTERVAL 1000     // Cresce a cada 1000ms
#define GAME_OVER_ANIMATION_TIME 2000  // Duração da animação de game over

typedef struct {
    uint8_t x;
    uint8_t y;
} Position;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct {
    Position segments[MAX_SNAKE_LENGTH];
    uint8_t length;
    Direction direction;
} Snake;

typedef struct {
    Snake snake;
    uint16_t score;
    uint8_t game_over;
    uint16_t growth_timer;
    uint16_t move_speed_ms;
    uint16_t game_over_timer;
} Game;

void game_init(Game* game);
void game_update(Game* game);
Direction get_joystick_direction(void);
uint8_t check_collision(const Snake* snake);
void draw_game(const Game* game);

#endif // SNAKE_H
