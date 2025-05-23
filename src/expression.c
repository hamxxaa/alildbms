#include "expression.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

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
        case TOKEN_STAR: return OP_MUL;
        case TOKEN_DIV: return OP_DIV;
        case TOKEN_ADD: return OP_ADD;
        case TOKEN_SUB: return OP_SUB;
        case TOKEN_NOT: return OP_NOT;
        default: return -1;
    }
}

static int get_precedence(Operator op) {
    switch (op) {
        case OP_MUL: case OP_DIV: return 6;
        case OP_ADD: case OP_SUB: return 5;
        case OP_EQ: case OP_NE: case OP_LT: case OP_LE: case OP_GT: case OP_GE: return 4;
        case OP_NOT: return 3;
        case OP_AND: return 2;
        case OP_OR: return 1;
        default: return 0;
    }
}

static Expression *parse_primary(Token *tokens, int *i);

static Expression *parse_binary_op_rhs(Token *tokens, int *i, int min_prec, Expression *lhs) {
    while (1) {
        Operator op = token_to_operator(tokens[*i].type);
        int prec = get_precedence(op);
        if (prec < min_prec) break;

        (*i)++;
        Expression *rhs = parse_primary(tokens, i);

        Operator next_op = token_to_operator(tokens[*i].type);
        if (get_precedence(next_op) > prec) {
            rhs = parse_binary_op_rhs(tokens, i, prec + 1, rhs);
        }

        Expression *parent = malloc(sizeof(Expression));
        parent->type = EXPR_BINARY;
        parent->binary.op = op;
        parent->binary.left = lhs;
        parent->binary.right = rhs;

        lhs = parent;
    }
    return lhs;
}

static Expression *parse_primary(Token *tokens, int *i) {
    if (tokens[*i].type == TOKEN_NOT) {
        (*i)++;
        Expression *child = parse_primary(tokens, i);
        Expression *expr = malloc(sizeof(Expression));
        expr->type = EXPR_UNARY;
        expr->unary.op = OP_NOT;
        expr->unary.child = child;
        return expr;
    }

    if (tokens[*i].type == TOKEN_OPEN_PARENTHESIS) {
        (*i)++;
        Expression *expr = parse_expression(tokens, i, -1);
        if (tokens[*i].type == TOKEN_CLOSE_PARENTHESIS)
            (*i)++;
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

Expression *parse_expression(Token *tokens, int *i, int count) {
    Expression *lhs = parse_primary(tokens, i);
    return parse_binary_op_rhs(tokens, i, 1, lhs);
}

int evaluate_expression(Expression *expr, const Table *table, const char *row_data) {
    switch (expr->type) {
        case EXPR_LITERAL:
            return atoi(expr->literal.value);

        case EXPR_COLUMN: {
            int offset = 0;
            for (int i = 0; i < table->columns_count; i++) {
                Column *col = &table->columns[i];
                if (strcmp(col->name, expr->column_name) == 0) {
                    if (col->type == INT) {
                        return *(int *)(row_data + offset);
                    } else {
                        // if needed: implement strcmp here
                        return 0;
                    }
                }
                offset += (col->type == INT) ? sizeof(int) : col->lenght + 1;
            }
            return 0;
        }

        case EXPR_UNARY:
            if (expr->unary.op == OP_NOT)
                return !evaluate_expression(expr->unary.child, table, row_data);
            break;

        case EXPR_BINARY: {
            int left = evaluate_expression(expr->binary.left, table, row_data);
            int right = evaluate_expression(expr->binary.right, table, row_data);
            switch (expr->binary.op) {
                case OP_ADD: return left + right;
                case OP_SUB: return left - right;
                case OP_MUL: return left * right;
                case OP_DIV: return right != 0 ? left / right : 0;
                case OP_EQ: return left == right;
                case OP_NE: return left != right;
                case OP_LT: return left < right;
                case OP_LE: return left <= right;
                case OP_GT: return left > right;
                case OP_GE: return left >= right;
                case OP_AND: return left && right;
                case OP_OR: return left || right;
                default: return 0;
            }
        }
    }
    return 0;
}

void free_expression(Expression *expr) {
    if (!expr) return;
    switch (expr->type) {
        case EXPR_LITERAL:
            free(expr->literal.value);
            break;
        case EXPR_COLUMN:
            free(expr->column_name);
            break;
        case EXPR_BINARY:
            free_expression(expr->binary.left);
            free_expression(expr->binary.right);
            break;
        case EXPR_UNARY:
            free_expression(expr->unary.child);
            break;
    }
    free(expr);
}
