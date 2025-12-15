#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// constants
#define MAX_TOKEN_SIZE 20
#define MAX_EXP_SIZE 1000
#define MAX_PARAMETERS 2

#define PI 3.141592653589793
#define E 2.718281828459045

// token types
typedef enum {
    TOK_INT,
    TOK_FLOAT,
    TOK_FUNC,
    TOK_SYMBOL,
    TOK_NEGATE,
    TOK_CONST,
    TOK_UNKNOWN
} TokenType;

typedef enum {
    FUNC_PI,
    FUNC_E,
    FUNC_SQRT,
    FUNC_POW,
    FUNC_SIN,
    FUNC_COS,
    FUNC_TAN,
    FUNC_ASIN,
    FUNC_ACOS,
    FUNC_ATAN,
    FUNC_EXP,
    FUNC_LN,
    FUNC_LOG2,
    FUNC_LOG10,
    FUNC_EMPTY
} FunctionId;

typedef struct {
    TokenType type;
    FunctionId func;
    char value[MAX_TOKEN_SIZE];
} Token;

typedef struct TreeNode {
    Token token;
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* pars[MAX_PARAMETERS];
    double f_value;
} TreeNode;

// parser context
typedef struct {
    char expression[MAX_EXP_SIZE];
    size_t length;
    size_t index;
    Token nextToken;
} ParserCtx;

// forward decl
static TreeNode* parseE(ParserCtx* ctx);
static TreeNode* parseT(ParserCtx* ctx);
static TreeNode* parseF(ParserCtx* ctx);
static TreeNode* parseFunction(ParserCtx* ctx);
static int scanToken(ParserCtx* ctx);

// ===== utility
static size_t str_len(const char* s);
static void str_copy(const char* src, char* dst, size_t max);
static int str_eq(const char* a, const char* b);

// ===== tree handling
static TreeNode* node_new(Token tok, TreeNode* l, TreeNode* r);
static void node_free(TreeNode* n);
static double node_eval(TreeNode* n);

// ===== entry
double parse(const char* expr);
