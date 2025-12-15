#include "keymap.h"
#include "expression.h"
#include <unistd.h>

/*
layer 0
" S " |  " <- " | " -> " | " D  "
" ( " |  "  ) " | " /  " | " AC "
" 1 " |  "  2 " | "  3 " | " *  "
" 4 " |  "  5 " | "  6 " | " +  "
" 7 " |  "  8 " | "  9 " | " -  "
" 0 " |  "  . " | "ANS " | " =  "
*/

/*
layer 1
" S  " |  " <- " | " -> " | " D  "
" (  " |  "  ) " | " /  " | " AC "
"cos " |  " sin" | " tan" | " *  "
"acos" |  "asin" | "atan" | " +  "
"log " |  "sqrt" | "exp " | " -  "
" 0  " |  "  . " | "ANS " | " =  "
*/


static KBD kbd_sing = {
    .keymaps = {
        // layer 0
        {
            { { KEY_TYPE_CHAR, "0", NULL}, { KEY_TYPE_CHAR, ".", NULL }, { KEY_TYPE_CHAR, "PI", NULL }, { KEY_TYPE_FUNC, "", func_evaluate } },        // row0
            { { KEY_TYPE_CHAR, "7", NULL}, { KEY_TYPE_CHAR, "8", NULL }, { KEY_TYPE_CHAR, "9", NULL }, { KEY_TYPE_CHAR, "-", NULL } },          // row1
            { { KEY_TYPE_CHAR, "4", NULL}, { KEY_TYPE_CHAR, "5", NULL }, { KEY_TYPE_CHAR, "6", NULL }, { KEY_TYPE_CHAR, "+", NULL } },          // row2
            { { KEY_TYPE_CHAR, "1", NULL}, { KEY_TYPE_CHAR, "2", NULL }, { KEY_TYPE_CHAR, "3", NULL }, { KEY_TYPE_CHAR, "*", NULL } },          // row3
            { { KEY_TYPE_CHAR, "(", NULL}, { KEY_TYPE_CHAR, ")", NULL }, { KEY_TYPE_CHAR, "/", NULL }, { KEY_TYPE_FUNC, "", func_clear } },       // row4
            { { KEY_TYPE_FUNC, "", func_switch_layer }, { KEY_TYPE_FUNC, "", func_mleft }, { KEY_TYPE_FUNC, "", func_mright }, { KEY_TYPE_FUNC, "", func_backspace } }, // row5
        },
        // layer 1
        {
            { { KEY_TYPE_CHAR, "0", NULL}, { KEY_TYPE_CHAR, ".", NULL }, { KEY_TYPE_FUNC, "", NULL }, { KEY_TYPE_FUNC, "", func_evaluate } },            // row0
            { { KEY_TYPE_CHAR, "log", NULL}, { KEY_TYPE_CHAR, "sqrt(", NULL }, { KEY_TYPE_CHAR, "exp(", NULL }, { KEY_TYPE_CHAR, "-", NULL } },     // row1
            { { KEY_TYPE_CHAR, "acos(", NULL}, { KEY_TYPE_CHAR, "asin(", NULL }, { KEY_TYPE_CHAR, "atan(", NULL }, { KEY_TYPE_CHAR, "+", NULL } },  // row2
            { { KEY_TYPE_CHAR, "cos(", NULL}, { KEY_TYPE_CHAR, "sin(", NULL }, { KEY_TYPE_CHAR, "tan(", NULL }, { KEY_TYPE_CHAR, "*", NULL } },     // row3
            { { KEY_TYPE_CHAR, "(", NULL}, { KEY_TYPE_CHAR, ")", NULL }, { KEY_TYPE_CHAR, "/", NULL }, { KEY_TYPE_FUNC, "", func_clear } },           // row4
            { { KEY_TYPE_FUNC, "", func_switch_layer }, { KEY_TYPE_FUNC, "", func_mleft }, { KEY_TYPE_FUNC, "", func_mright }, { KEY_TYPE_FUNC, "", func_backspace } }, // row5
        }
    },
    .action_layer = 0,
};

void func_switch_layer() { kbd_sing.action_layer = (kbd_sing.action_layer + 1 ) % NUM_LAYERS; }
void func_evaluate(void) { printf("result of %s = %fd\n", expr_get_str(expr_get()), expr_evaluate(expr_get())); }
void func_backspace() { expr_backspace(expr_get()); }
void func_mleft() { expr_mleft(expr_get()); }
void func_mright() { expr_mright(expr_get()); }
void func_clear() { expr_clear(expr_get()); }

key_action_t get_action(int row, int col) {
    return kbd_sing.keymaps[kbd_sing.action_layer][row][col];
}
