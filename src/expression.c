#include "expression.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int token_to_operator(TokenType type)
{
    switch (type)
    {
    case TOKEN_EQ:
        return OP_EQ;
    case TOKEN_NE:
        return OP_NE;
    case TOKEN_LT:
        return OP_LT;
    case TOKEN_LE:
        return OP_LE;
    case TOKEN_GT:
        return OP_GT;
    case TOKEN_GE:
        return OP_GE;
    case TOKEN_AND:
        return OP_AND;
    case TOKEN_OR:
        return OP_OR;
    case TOKEN_STAR:
        return OP_MUL;
    case TOKEN_DIV:
        return OP_DIV;
    case TOKEN_ADD:
        return OP_ADD;
    case TOKEN_SUB:
        return OP_SUB;
    case TOKEN_NOT:
        return OP_NOT;
    default:
        return -1;
    }
}

static int get_precedence(Operator op)
{
    switch (op)
    {
    case OP_MUL:
    case OP_DIV:
        return 6;
    case OP_ADD:
    case OP_SUB:
        return 5;
    case OP_EQ:
    case OP_NE:
    case OP_LT:
    case OP_LE:
    case OP_GT:
    case OP_GE:
        return 4;
    case OP_NOT:
        return 3;
    case OP_AND:
        return 2;
    case OP_OR:
        return 1;
    default:
        return 0;
    }
}

static Expression *parse_primary(Token *tokens, int *i);

static Expression *parse_binary_op_rhs(Token *tokens, int *i, int min_prec, Expression *lhs)
{
    while (tokens[*i].type != TOKEN_EOF &&
           tokens[*i].type != TOKEN_SEMICOLON &&
           tokens[*i].type != TOKEN_CLOSE_PARENTHESIS)
    {

        int op = token_to_operator(tokens[*i].type);
        int prec = get_precedence(op);

        if (op == -1 || prec < min_prec)
            break;

        (*i)++; // consume the operator

        Expression *rhs = parse_primary(tokens, i);

        if (!rhs)
            return lhs;

        int next_op = token_to_operator(tokens[*i].type);
        int next_prec = get_precedence(next_op);

        if (next_op != -1 && next_prec > prec)
        {
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

static Expression *parse_primary(Token *tokens, int *i)
{
    if (tokens[*i].type == TOKEN_EOF)
        return NULL;

    if (tokens[*i].type == TOKEN_NOT)
    {
        (*i)++;
        Expression *child = parse_primary(tokens, i);
        if (!child)
            return NULL;
        Expression *expr = malloc(sizeof(Expression));
        expr->type = EXPR_UNARY;
        expr->unary.op = OP_NOT;
        expr->unary.child = child;
        return expr;
    }

    if (tokens[*i].type == TOKEN_OPEN_PARENTHESIS)
    {
        (*i)++;
        Expression *expr = parse_expression(tokens, i, -1);
        if (tokens[*i].type == TOKEN_CLOSE_PARENTHESIS)
        {
            (*i)++;
        }
        else
        {
            printf("Error: Missing closing parenthesis\n");
        }
        return expr;
    }

    Expression *expr = malloc(sizeof(Expression));
    if (tokens[*i].type == TOKEN_IDENTIFIER)
    {
        if (tokens[*i + 1].type == TOKEN_DOT)
        {
            expr->type = EXPR_ALIAS_COLUMN;
            expr->alias_column.alias = strdup(tokens[*i].token);
            (*i) += 2; // skip .
            if (tokens[*i].type != TOKEN_IDENTIFIER)
            {
                printf("Error: Expected column name after %s\n", expr->alias_column.alias);
                free(expr);
                return NULL;
            }
            expr->alias_column.column_name = strdup(tokens[*i].token);
        }
        else
        {
            expr->type = EXPR_COLUMN;
            expr->column_name = strdup(tokens[*i].token);
        }
    }
    else if (tokens[*i].type == TOKEN_STRING || tokens[*i].type == TOKEN_NUMBER)
    {
        expr->type = EXPR_LITERAL;
        expr->literal.value = strdup(tokens[*i].token);
        expr->literal.is_string = (tokens[*i].type == TOKEN_STRING);
    }
    else
    {
        free(expr);
        return NULL;
    }
    (*i)++;
    return expr;
}

Expression *parse_expression(Token *tokens, int *i, int count)
{
    (void)count;
    Expression *lhs = parse_primary(tokens, i);
    if (!lhs)
        return NULL;
    return parse_binary_op_rhs(tokens, i, 1, lhs);
}

static void *evaluate_column(Expression *expr, Table *tables[], char *row_datas[], int table_count, DataType *type) // caller is responsible for freeing the return value
{
    int check = 0;
    int table_index;
    for (int i = 0; i < table_count; i++)
    {
        if (check_column_exists_by_name(tables[i], expr->column_name))
        {
            check++;
            table_index = i;
        }
        if (check > 1)
        {
            printf("Error: Column %s exists in more than one table, give specifications\n", expr->column_name);
            return NULL;
        }
    }
    Column *col = get_column(tables[table_index], expr->column_name);
    if (col->type == INT)
    {
        *type = INT;
        int *val = malloc(sizeof(int));
        *val = *(int *)(row_datas[table_index] + calculate_offset(tables[table_index], *col));
        return val;
    }
    else
    {
        *type = STRING;
        char *val = malloc(col->lenght + 1);
        strncpy(val, row_datas[table_index] + calculate_offset(tables[table_index], *col), col->lenght);
        val[col->lenght] = '\0';
        return val;
    }
}

void *evaluate_alias_column(Expression *expr, Table *tables[], char *alias[], char *row_datas[], int table_count, DataType *type)
{
    int check = 0;
    int table_index;

    for (int i = 0; i < table_count; i++)
    {
        if (strcmp(expr->alias_column.alias, alias[i]) == 0)
        {
            check = 1;
            table_index = i;
        }
    }
    if (check == 0)
    {
        printf("Error: Alias %s does not exist\n", expr->alias_column.alias);
        return NULL;
    }

    Column *col = get_column(tables[table_index], expr->alias_column.column_name);
    if (col->type == INT)
    {
        *type = INT;
        int *val = malloc(sizeof(int));
        *val = *(int *)(row_datas[table_index] + calculate_offset(tables[table_index], *col));
        return val;
    }
    else
    {
        *type = STRING;
        char *val = malloc(col->lenght + 1);
        strncpy(val, row_datas[table_index] + calculate_offset(tables[table_index], *col), col->lenght);
        val[col->lenght] = '\0';
        return val;
    }
}

int evaluate_expression(Expression *expr, Table *tables[], char *alias[], char *row_datas[], int table_count)
{
    switch (expr->type)
    {
    case EXPR_LITERAL:
        if (expr->literal.is_string)
            return -1; // string literals are handled specially in EXPR_BINARY
        return atoi(expr->literal.value);

    case EXPR_ALIAS_COLUMN:
        DataType type;

        void *val = evaluate_alias_column(expr, tables, alias, row_datas, table_count, &type);
        if (val == NULL)
        {
            return -1;
        }
        switch (type)
        {
        case INT:
            int r = *(int *)val;
            free(val);
            return r;
            break;

        case STRING:
            free(val);
            return -1;
            break;
        }
        break;
    case EXPR_COLUMN:
    {
        DataType type;
        void *val = evaluate_column(expr, tables, row_datas, table_count, &type);
        if (val == NULL)
        {
            return -1;
        }
        switch (type)
        {
        case INT:
            int r = *(int *)val;
            free(val);
            return r;
            break;

        case STRING:
            free(val);
            return -1;
            break;
        }
        break;
    }

    case EXPR_UNARY:
        if (expr->unary.op == OP_NOT)
            return !evaluate_expression(expr->unary.child, tables, alias, row_datas, table_count);
        break;

    case EXPR_BINARY:
    {
        // These hold flags and data for string vs int mode
        int is_left_str = 0, is_right_str = 0;
        char left_str[256] = {0}, right_str[256] = {0};
        int left = 0, right = 0;

        // LEFT SIDE
        if (expr->binary.left->type == EXPR_LITERAL && expr->binary.left->literal.is_string)
        {
            is_left_str = 1;
            strncpy(left_str, expr->binary.left->literal.value, 255);
        }
        else if (expr->binary.left->type == EXPR_COLUMN)
        {
            DataType type;
            void *val = evaluate_column(expr->binary.left, tables, row_datas, table_count, &type);
            switch (type)
            {
            case INT:
                left = *(int *)val;
                free(val);
                break;

            case STRING:
                is_left_str = 1;
                strcpy(left_str, val);
                free(val);
                break;
            }
        }
        else if (expr->binary.left->type == EXPR_ALIAS_COLUMN)
        {
            DataType type;
            void *val = evaluate_alias_column(expr->binary.left, tables, alias, row_datas, table_count, &type);
            switch (type)
            {
            case INT:
                left = *(int *)val;
                free(val);
                break;

            case STRING:
                is_left_str = 1;
                strcpy(left_str, val);
                free(val);
                break;
            }
        }
        else
        {
            left = evaluate_expression(expr->binary.left, tables, alias, row_datas, table_count);
        }

        // RIGHT SIDE
        if (expr->binary.right->type == EXPR_LITERAL)
        {
            if (expr->binary.right->literal.is_string)
            {
                is_right_str = 1;
                strncpy(right_str, expr->binary.right->literal.value, 255);
            }
            else
            {
                right = atoi(expr->binary.right->literal.value);
            }
        }
        else if (expr->binary.right->type == EXPR_COLUMN)
        {
            DataType type;
            void *val = evaluate_column(expr->binary.right, tables, row_datas, table_count, &type);
            switch (type)
            {
            case INT:
                right = *(int *)val;
                free(val);
                break;

            case STRING:
                is_right_str = 1;
                strcpy(right_str, val);
                free(val);
                break;
            }
        }
        else if (expr->binary.right->type == EXPR_ALIAS_COLUMN)
        {
            DataType type;
            void *val = evaluate_column(expr->binary.right, tables, row_datas, table_count, &type);
            switch (type)
            {
            case INT:
                right = *(int *)val;
                free(val);
                break;

            case STRING:
                is_right_str = 1;
                strcpy(right_str, val);
                free(val);
                break;
            }
        }
        else
        {
            right = evaluate_expression(expr->binary.right, tables, alias, row_datas, table_count);
        }

        // Handle string comparisons
        if (is_left_str && is_right_str)
        {
            int cmp = strcmp(left_str, right_str);
            switch (expr->binary.op)
            {
            case OP_EQ:
                return cmp == 0;
            case OP_NE:
                return cmp != 0;
            default:
                return 0; // Don't allow arithmetic ops on strings
            }
        }

        // Handle integer logic
        // If both sides are NOT strings, always evaluate integer logic
        if (!is_left_str && !is_right_str)
        {
            switch (expr->binary.op)
            {
            case OP_ADD:
                return left + right;
            case OP_SUB:
                return left - right;
            case OP_MUL:
                return left * right;
            case OP_DIV:
                return right != 0 ? left / right : 0;
            case OP_EQ:
                return left == right;
            case OP_NE:
                return left != right;
            case OP_LT:
                return left < right;
            case OP_LE:
                return left <= right;
            case OP_GT:
                return left > right;
            case OP_GE:
                return left >= right;
            case OP_AND:
                return left && right;
            case OP_OR:
                return left || right;
            default:
                return 0;
            }
        }
    }
    }
    return 0;
}

void free_expression(Expression *expr)
{
    if (!expr)
        return;
    switch (expr->type)
    {
    case EXPR_LITERAL:
        free(expr->literal.value);
        break;
    case EXPR_COLUMN:
        free(expr->column_name);
        break;
    case EXPR_ALIAS_COLUMN:
        free(expr->alias_column.alias);
        free(expr->alias_column.column_name);
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
