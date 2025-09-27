#include "math_parser.h"

// ===== utility
static size_t str_len(const char* s) {
    return strlen(s);
}

static void str_copy(const char* src, char* dst, size_t max) {
    strncpy(dst, src, max - 1);
    dst[max - 1] = '\0';
}

static int str_eq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

// ===== tree handling
static TreeNode* node_new(Token tok, TreeNode* l, TreeNode* r) {
    TreeNode* n = (TreeNode*)malloc(sizeof(TreeNode));
    if (!n) return NULL;
    n->token = tok;
    n->left = l;
    n->right = r;
    for (int i = 0; i < MAX_PARAMETERS; i++) n->pars[i] = NULL;
    n->f_value = 0;
    if (tok.type == TOK_INT || tok.type == TOK_FLOAT) {
        n->f_value = atof(tok.value);
    }
    return n;
}

static void node_free(TreeNode* n) {
    if (!n) return;
    node_free(n->left);
    node_free(n->right);
    for (int i = 0; i < MAX_PARAMETERS; i++) node_free(n->pars[i]);
    free(n);
}

static double node_eval(TreeNode* n) {
    if (!n) return 0;
    switch (n->token.type) {
        case TOK_INT:
        case TOK_FLOAT:
            return n->f_value;
        case TOK_SYMBOL:
            switch (n->token.value[0]) {
                case '+': return node_eval(n->left) + node_eval(n->right);
                case '-':
                    if (n->right)
                        return node_eval(n->left) - node_eval(n->right);
                    else
                        return -node_eval(n->left);
                case '*': return node_eval(n->left) * node_eval(n->right);
                case '/': return node_eval(n->left) / node_eval(n->right);
                default: return 0;
            }
        case TOK_FUNC:
            switch (n->token.func) {
                case FUNC_SQRT:  return sqrt(node_eval(n->pars[0]));
                case FUNC_POW:   return pow(node_eval(n->pars[0]), node_eval(n->pars[1]));
                case FUNC_SIN:   return sin(node_eval(n->pars[0]));
                case FUNC_COS:   return cos(node_eval(n->pars[0]));
                case FUNC_TAN:   return tan(node_eval(n->pars[0]));
                case FUNC_ASIN:  return asin(node_eval(n->pars[0]));
                case FUNC_ACOS:  return acos(node_eval(n->pars[0]));
                case FUNC_ATAN:  return atan(node_eval(n->pars[0]));
                case FUNC_EXP:   return exp(node_eval(n->pars[0]));
                case FUNC_LN:    return log(node_eval(n->pars[0]));
                case FUNC_LOG2:  return log2(node_eval(n->pars[0]));
                case FUNC_LOG10: return log10(node_eval(n->pars[0]));
                default: return 0;
            }
        default:
            return 0;
    }
}

// ===== parser
static TreeNode* parseE(ParserCtx* ctx) {
    TreeNode* a = parseT(ctx);
    while (ctx->nextToken.type == TOK_SYMBOL &&
           (ctx->nextToken.value[0] == '+' || ctx->nextToken.value[0] == '-')) {
        Token t = ctx->nextToken;
        scanToken(ctx);
        TreeNode* b = parseT(ctx);
        a = node_new(t, a, b);
    }
    return a;
}

static TreeNode* parseT(ParserCtx* ctx) {
    TreeNode* a = parseF(ctx);
    while (ctx->nextToken.type == TOK_SYMBOL &&
           (ctx->nextToken.value[0] == '*' || ctx->nextToken.value[0] == '/')) {
        Token t = ctx->nextToken;
        scanToken(ctx);
        TreeNode* b = parseF(ctx);
        a = node_new(t, a, b);
    }
    return a;
}

static TreeNode* parseF(ParserCtx* ctx) {
    if (ctx->nextToken.type == TOK_INT || ctx->nextToken.type == TOK_FLOAT) {
        Token t = ctx->nextToken;
        scanToken(ctx);
        return node_new(t, NULL, NULL);
    }
    if (ctx->nextToken.type == TOK_SYMBOL && ctx->nextToken.value[0] == '(') {
        scanToken(ctx);
        TreeNode* a = parseE(ctx);
        if (ctx->nextToken.type == TOK_SYMBOL && ctx->nextToken.value[0] == ')') {
            scanToken(ctx);
            return a;
        }
    }
    if (ctx->nextToken.type == TOK_SYMBOL && ctx->nextToken.value[0] == '-') {
        Token t = ctx->nextToken;
        scanToken(ctx);
        return node_new(t, parseF(ctx), NULL);
    }
    return parseFunction(ctx);
}

static TreeNode* parseFunction(ParserCtx* ctx) {
    Token t = ctx->nextToken;
    scanToken(ctx);
    if (ctx->nextToken.type == TOK_SYMBOL && ctx->nextToken.value[0] == '(') {
        scanToken(ctx);
        TreeNode* f = node_new(t, NULL, NULL);
        int i = 0;
        f->pars[i++] = parseE(ctx);
        while (ctx->nextToken.type == TOK_SYMBOL && ctx->nextToken.value[0] == ',' && i < MAX_PARAMETERS) {
            scanToken(ctx);
            f->pars[i++] = parseE(ctx);
        }
        if (ctx->nextToken.type == TOK_SYMBOL && ctx->nextToken.value[0] == ')') {
            scanToken(ctx);
            return f;
        }
    }
    return NULL;
}

// ===== lexer
static int scanToken(ParserCtx* ctx) {
    while (ctx->index < ctx->length) {
        char ch = ctx->expression[ctx->index];
        if (isspace(ch)) { ctx->index++; continue; }

        // symbols
        if (strchr("()+-*/,", ch)) {
            ctx->nextToken.type = TOK_SYMBOL;
            ctx->nextToken.value[0] = ch;
            ctx->nextToken.value[1] = '\0';
            ctx->index++;
            return 0;
        }

        // numbers
        if (isdigit(ch)) {
            char buf[MAX_TOKEN_SIZE]; int k = 0;
            while (ctx->index < ctx->length &&
                  (isdigit((int)ctx->expression[ctx->index]) || ctx->expression[ctx->index] == '.')) {
                if (k < MAX_TOKEN_SIZE-1)
                    buf[k++] = ctx->expression[ctx->index];
                ctx->index++;
            }
            buf[k] = '\0';
            str_copy(buf, ctx->nextToken.value, MAX_TOKEN_SIZE);
            ctx->nextToken.type = strchr(buf, '.') ? TOK_FLOAT : TOK_INT;
            return 0;
        }

        // functions / identifiers
        if (isalpha((int)ch)) {
            char buf[MAX_TOKEN_SIZE]; int k = 0;
            while (ctx->index < ctx->length &&
                   (isalnum((int)ctx->expression[ctx->index]) || ctx->expression[ctx->index] == '_')) {
                if (k < MAX_TOKEN_SIZE-1)
                    buf[k++] = ctx->expression[ctx->index];
                ctx->index++;
            }
            buf[k] = '\0';
            str_copy(buf, ctx->nextToken.value, MAX_TOKEN_SIZE);
            ctx->nextToken.type = TOK_FUNC;
            if      (str_eq(buf,"sqrt")) ctx->nextToken.func = FUNC_SQRT;
            else if (str_eq(buf,"pow"))  ctx->nextToken.func = FUNC_POW;
            else if (str_eq(buf,"sin"))  ctx->nextToken.func = FUNC_SIN;
            else if (str_eq(buf,"cos"))  ctx->nextToken.func = FUNC_COS;
            else if (str_eq(buf,"tan"))  ctx->nextToken.func = FUNC_TAN;
            else if (str_eq(buf,"asin")) ctx->nextToken.func = FUNC_ASIN;
            else if (str_eq(buf,"acos")) ctx->nextToken.func = FUNC_ACOS;
            else if (str_eq(buf,"atan")) ctx->nextToken.func = FUNC_ATAN;
            else if (str_eq(buf,"exp"))  ctx->nextToken.func = FUNC_EXP;
            else if (str_eq(buf,"ln"))   ctx->nextToken.func = FUNC_LN;
            else if (str_eq(buf,"log2")) ctx->nextToken.func = FUNC_LOG2;
            else if (str_eq(buf,"log10"))ctx->nextToken.func = FUNC_LOG10;
            else                         ctx->nextToken.func = FUNC_EMPTY;
            return 0;
        }

        return 1; // unknown char
    }
    return 2; // end
}

// ===== entry
double parse(const char* expr) {
    ParserCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    str_copy(expr, ctx.expression, MAX_EXP_SIZE);
    ctx.length = str_len(ctx.expression);
    ctx.index = 0;
    scanToken(&ctx);

    TreeNode* root = parseE(&ctx);
    double val = node_eval(root);
    node_free(root);
    return val;
}
