#include "expression.h"
#include <string.h>

extern double parse(const char* expr_str);

static Expression expr_sing = {.len = 0, .cursor = 0, .buf[0] = '\0'};

void expr_init(Expression *expr) {
    expr->len = 0;
    expr->cursor = 0;
    expr->buf[0] = '\0';
    expr->result = 0;
}

void expr_clear(Expression *expr) {
    expr_init(expr);
}

Expression *expr_get() {
    return &expr_sing;
}

const char *expr_get_str(Expression *expr) {
    expr->buf[expr->len] = '\0';
    return expr->buf;
}

void expr_insert(Expression *e, const char *s) {
    int n = strlen(s);
    if (e->len + n >= sizeof(e->buf)) return; // full

    // move tail to make room
    memmove(e->buf + e->cursor + n, e->buf + e->cursor, e->len - e->cursor + 1);
    memcpy(e->buf + e->cursor, s, n);
    e->len += n;
    e->cursor += n;
}

void expr_backspace(Expression *e) {
    if (e->cursor == 0) return;
    memmove(e->buf + e->cursor - 1, e->buf + e->cursor, e->len - e->cursor + 1);
    e->len--;
    e->cursor--;
}

void expr_mleft(Expression *e) {
    if (e->cursor > 0) e->cursor--;
}

void expr_mright(Expression *e) {
    if (e->cursor < e->len) e->cursor++;
}

double expr_evaluate(Expression *e) {
    e->buf[e->len] = '\0';
    e->result = parse(e->buf);
    return e->result;
}
