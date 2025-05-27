#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "sql_tokenizer.h"
#include "table.h"

typedef struct Token Token; // Forward declaration of Token struct

typedef enum
{
    EXPR_LITERAL,
    EXPR_COLUMN,
    EXPR_ALIAS_COLUMN,
    EXPR_BINARY,
    EXPR_UNARY
} ExprType;

typedef enum
{
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_NOT
} Operator;

typedef struct Expression
{
    ExprType type;
    union
    {
        struct
        {
            char *value;
            int is_string;
        } literal;

        char *column_name;

        struct
        {
            char *alias;
            char *column_name;
        } alias_column;

        struct
        {
            Operator op;
            struct Expression *left;
            struct Expression *right;
        } binary;

        struct
        {
            Operator op;
            struct Expression *child;
        } unary;
    };
} Expression;

/**
 * Parses an expression using tokens and returns an expression tree.
 *
 * @param tokens The array of tokens.
 * @param i Pointer to the current index in the token stream.
 * @param count The number of tokens (can be -1 if unused).
 * @return Expression* pointer to the root of the expression tree.
 */
Expression *parse_expression(Token *tokens, int *i, int count);

/**
 * Evaluates the expression tree for a specific row.
 *
 * @param expr The expression tree to evaluate.
 * @param table The table the row belongs to.
 * @param row_data The pointer to the raw binary row data.
 * @return int 1 if expression is true, 0 if false.
 */
int evaluate_expression(Expression *expr, Table *tables[], char *alias[], char *row_datas[], int table_count);

/**
 * Frees all memory used by the expression tree.
 *
 * @param expr The root node of the expression tree.
 */
void free_expression(Expression *expr);

#endif
