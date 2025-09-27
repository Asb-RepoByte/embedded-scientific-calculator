#pragma once

#include <stdint.h>
#define NUM_LAYERS 2

#define ROWS_NUM 6
#define COLS_NUM 4

typedef enum {
    KEY_TYPE_CHAR,
    KEY_TYPE_STRING,
    KEY_TYPE_FUNC,
} key_type_t;

typedef void (*key_func_t)(void) ;

typedef struct {
    key_type_t type;
    const char *output;
    key_func_t func;

} key_action_t;

void func_switch_layer(void);
void func_evaluate(void);
void func_clear(void);
void func_backspace(void);
void func_mleft(void);
void func_mright(void);

key_action_t get_action(int row, int col);

typedef struct {
    key_action_t keymaps[NUM_LAYERS][ROWS_NUM][COLS_NUM];
    uint8_t action_layer;

} KBD;
