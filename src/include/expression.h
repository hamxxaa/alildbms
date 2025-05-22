#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "sql_tokenizer.h"
#include "table.h"

typedef enum {
    EXPR_LITERAL,
    EXPR_COLUMN,
    EXPR_BINARY,
    EXPR_UNARY
} ExprType;

typedef enum {
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_AND, OP_OR, OP_NOT
} Operator;

typedef struct Expression {
    ExprType type;
    union {
        struct {
            char *value;
            int is_string;
        } literal;

        char *column_name;

        struct {
            Operator op;
            struct Expression *left;
            struct Expression *right;
        } binary;

        struct {
            Operator op;
            struct Expression *child;
        } unary;
    };
} Expression;

Expression *parse_expression(Token *tokens, int *i, int count);
int evaluate_expression(Expression *expr, const Table *table, const char *row_data);
void free_expression(Expression *expr);

#endif
