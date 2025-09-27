#pragma once

#include "math_parser.h"

#define MAX_EXP_SIZE 1000

typedef struct {
    char buf[MAX_EXP_SIZE];
    int cursor;
    int len;
    double result;
} Expression ;

void expr_init(Expression *expr);
void expr_clear(Expression *expr);
void expr_insert(Expression *expr, const char *s);
void expr_mleft(Expression *expr);
void expr_mright(Expression *expr);
void expr_backspace(Expression *expr);
double expr_evaluate(Expression *expr);
Expression *expr_get();
const char *expr_get_str(Expression *expr);
