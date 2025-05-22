#include "expression.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Operator token_to_operator(TokenType type) {
    switch (type) {
        case TOKEN_EQ: return OP_EQ;
        case TOKEN_NE: return OP_NE;
        case TOKEN_LT: return OP_LT;
        case TOKEN_LE: return OP_LE;
        case TOKEN_GT: return OP_GT;
        case TOKEN_GE: return OP_GE;
        case TOKEN_AND: return OP_AND;
        case TOKEN_OR: return OP_OR;
        default: return -1;
    }
}

static int get_precedence(Operator op) {
    switch (op) {
        case OP_OR: return 1;
        case OP_AND: return 2;
        case OP_EQ: case OP_NE: case OP_LT: case OP_LE: case OP_GT: case OP_GE: return 3;
        case OP_ADD: case OP_SUB: return 4;
        case OP_MUL: case OP_DIV: return 5;
        case OP_NOT: return 6;
        default: return 0;
    }
}

static Expression *parse_primary(Token *tokens, int *i) {
    if (tokens[*i].type == TOKEN_OPEN_PARENTHESIS) {
        (*i)++;
        Expression *expr = parse_expression(tokens, i, -1);
        (*i)++; // skip )
        return expr;
    }

    Expression *expr = malloc(sizeof(Expression));
    if (tokens[*i].type == TOKEN_IDENTIFIER) {
        expr->type = EXPR_COLUMN;
        expr->column_name = strdup(tokens[*i].token);
    } else if (tokens[*i].type == TOKEN_STRING || tokens[*i].type == TOKEN_NUMBER) {
        expr->type = EXPR_LITERAL;
        expr->literal.value = strdup(tokens[*i].token);
        expr->literal.is_string = (tokens[*i].type == TOKEN_STRING);
    }
    (*i)++;
    return expr;
}

static Expression *parse_binary_op_rhs(Token *tokens, int *i, int min_prec, Expression *lhs) {
    while (1) {
        Operator op = token_to_operator(tokens[*i].type);
        int prec = get_precedence(op);
        if (prec < min_prec) break;

        (*i)++;
        Expression *rhs = parse_primary(tokens, i);
        Operator next_op = token_to_operator(tokens[*i].type);
        if (get_precedence(next_op) > prec)
            rhs = parse_binary_op_rhs(tokens, i, prec + 1, rhs);

        Expression *parent = malloc(sizeof(Expression));
        parent->type = EXPR_BINARY;
        parent->binary.op = op;
        parent->binary.left = lhs;
        parent->binary.right = rhs;
        lhs = parent;
    }
    return lhs;
}

Expression *parse_expression(Token *tokens, int *i, int count) {
    Expression *lhs = parse_primary(tokens, i);
    return parse_binary_op_rhs(tokens, i, 1, lhs);
}
