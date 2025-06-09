#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

#define BOARD_SIZE 8
#define MAX_SNAKE_LENGTH 64

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
    Position food;
    uint16_t score;
    uint8_t game_over;
} Game;

void game_init(Game* game);
void game_update(Game* game);
Direction get_joystick_direction(void);
void spawn_food(Game* game);
uint8_t check_collision(const Snake* snake);
void draw_game(const Game* game);

#endif // SNAKE_H
