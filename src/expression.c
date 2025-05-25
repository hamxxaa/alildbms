#include "expression.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int token_to_operator(TokenType type) {
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
    while (tokens[*i].type != TOKEN_EOF &&
           tokens[*i].type != TOKEN_SEMICOLON &&
           tokens[*i].type != TOKEN_CLOSE_PARENTHESIS) {

        int op = token_to_operator(tokens[*i].type);
        int prec = get_precedence(op);

        if (op == -1 || prec < min_prec) break;

        (*i)++;  // consume the operator

        Expression *rhs = parse_primary(tokens, i);


        if (!rhs) return lhs;

        int next_op = token_to_operator(tokens[*i].type);
        int next_prec = get_precedence(next_op);

        if (next_op != -1 && next_prec > prec) {
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

Expression *parse_expression(Token *tokens, int *i, int count);

static Expression *parse_primary(Token *tokens, int *i) {
    if (tokens[*i].type == TOKEN_EOF)
        return NULL;

    if (tokens[*i].type == TOKEN_NOT) {
        (*i)++;
        Expression *child = parse_primary(tokens, i);
        if (!child) return NULL;
        Expression *expr = malloc(sizeof(Expression));
        expr->type = EXPR_UNARY;
        expr->unary.op = OP_NOT;
        expr->unary.child = child;
        return expr;
    }

    if (tokens[*i].type == TOKEN_OPEN_PARENTHESIS) {
        (*i)++;
        Expression *expr = parse_expression(tokens, i, -1);
        if (tokens[*i].type == TOKEN_CLOSE_PARENTHESIS) {
            (*i)++;
        } else {
            printf("Error: Missing closing parenthesis\n");
        }
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
    } else {
        free(expr);
        return NULL;
    }
    (*i)++;
    return expr;
}

Expression *parse_expression(Token *tokens, int *i, int count) {
    (void)count;
    Expression *lhs = parse_primary(tokens, i);
    if (!lhs) return NULL;
    return parse_binary_op_rhs(tokens, i, 1, lhs);
}

int evaluate_expression(Expression *expr, const Table *table, const char *row_data) {
    switch (expr->type) {
        case EXPR_LITERAL:
            if (expr->literal.is_string)
                return -1; // string literals are handled specially in EXPR_BINARY
            return atoi(expr->literal.value);

        case EXPR_COLUMN: {
            int offset = 0;
            for (int i = 0; i < table->columns_count; i++) {
                const Column *col = &table->columns[i];
                if (strcmp(col->name, expr->column_name) == 0) {
                    if (col->type == INT) {
                        return *(int *)(row_data + offset);
                    } else {
                        return -1; // signal that string data is not directly evaluatable
                    }
                }
                offset += (col->type == INT) ? (int)sizeof(int) : col->lenght + 1;
            }
            return 0;
        }

        case EXPR_UNARY:
            if (expr->unary.op == OP_NOT)
                return !evaluate_expression(expr->unary.child, table, row_data);
            break;

        case EXPR_BINARY: {
            // These hold flags and data for string vs int mode
            int is_left_str = 0, is_right_str = 0;
            char left_str[256] = {0}, right_str[256] = {0};
            int left = 0, right = 0;

            // LEFT SIDE
            if (expr->binary.left->type == EXPR_LITERAL && expr->binary.left->literal.is_string) {
                is_left_str = 1;
                strncpy(left_str, expr->binary.left->literal.value, 255);
            } else if (expr->binary.left->type == EXPR_COLUMN) {
                int offset = 0;
                for (int i = 0; i < table->columns_count; i++) {
                    const Column *col = &table->columns[i];
                    if (strcmp(col->name, expr->binary.left->column_name) == 0) {
                        if (col->type == STRING) {
                            is_left_str = 1;
                            strncpy(left_str, row_data + offset, col->lenght);
                            left_str[col->lenght] = '\0';
                        } else {
                            left = *(int *)(row_data + offset);
                        }
                        break;
                    }
                    offset += (table->columns[i].type == INT) ? (int)sizeof(int) : table->columns[i].lenght + 1;
                }
            } else {
                left = evaluate_expression(expr->binary.left, table, row_data);
            }

            // RIGHT SIDE
            if (expr->binary.right->type == EXPR_LITERAL && expr->binary.right->literal.is_string) {
                is_right_str = 1;
                strncpy(right_str, expr->binary.right->literal.value, 255);
            } else if (expr->binary.right->type == EXPR_COLUMN) {
                int offset = 0;
                for (int i = 0; i < table->columns_count; i++) {
                    const Column *col = &table->columns[i];
                    if (strcmp(col->name, expr->binary.right->column_name) == 0) {
                        if (col->type == STRING) {
                            is_right_str = 1;
                            strncpy(right_str, row_data + offset, col->lenght);
                            right_str[col->lenght] = '\0';
                        } else {
                            right = *(int *)(row_data + offset);
                        }
                        break;
                    }
                    offset += (table->columns[i].type == INT) ? (int)sizeof(int) : table->columns[i].lenght + 1;
                }
            } else {
                right = evaluate_expression(expr->binary.right, table, row_data);
            }

            // Handle string comparisons
            if (is_left_str && is_right_str) {
                int cmp = strcmp(left_str, right_str);
                switch (expr->binary.op) {
                    case OP_EQ: return cmp == 0;
                    case OP_NE: return cmp != 0;
                    default: return 0; // Don't allow arithmetic ops on strings
                }
            }

            // Handle integer logic
            // If both sides are NOT strings, always evaluate integer logic
            if (!is_left_str && !is_right_str) {
                printf("[EVAL] operator: %d, left: %d, right: %d\n", expr->binary.op, left, right);
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
