#include "sql_tokenizer.h"
#include "table.h"
#include "file_io.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void nested_loop_join(Expression *expr, Table *tables[], char *alias[], FILE *files[], int table_count, char *rows[], int current_table, long return_positions[][table_count], int *match_count /*, Column *columns[], char *column_alias[], int column_count*/)
{
    if (current_table >= table_count)
    {
        if (evaluate_expression(expr, tables, alias, rows, table_count))
        {
            for (int i = 0; i < table_count; i++)
            {
                return_positions[*match_count][i] = ftell(files[i]) - tables[i]->row_size_in_bytes;
            }
            (*match_count)++;
        }
        return;
    }

    while (fread(rows[current_table], tables[current_table]->row_size_in_bytes, 1, files[current_table]) == 1)
    {
        if (isfree(tables[current_table], ftell(files[current_table]) - tables[current_table]->row_size_in_bytes))
        {
            continue; // Skip free rows
        }
        // Recursively process next table
        nested_loop_join(expr, tables, alias, files, table_count, rows, current_table + 1, return_positions, match_count /*, columns, column_alias, column_count*/);

        // After processing all deeper tables, rewind them for next iteration
        for (int i = current_table + 1; i < table_count; i++)
        {
            rewind(files[i]);
        }
    }

    // Rewind current table for potential future joins
    rewind(files[current_table]);
}

Token *tokenize(const char *sql, int *out_count)
{
    Token *tokens = malloc(sizeof(Token) * MAX_TOKEN_COUNT);
    int token_count = 0;

    while (1)
    {
        if (token_count >= MAX_TOKEN_COUNT)
        {
            printf("Error: Too many tokens, Maximum is %d\n", MAX_TOKEN_COUNT);
            free(tokens);
            return NULL;
        }
        Token token = {TOKEN_ERROR, ""};

        while (isspace(*sql))
        {
            sql++;
        }

        if (*sql == '\0')
        {
            token.type = TOKEN_EOF;
            tokens[token_count++] = token;
            break;
        }
        if (strncmp(sql, "SELECT", 6) == 0)
        {
            token.type = TOKEN_SELECT;
            strncpy(token.token, sql, 6);
            token.token[6] = '\0';
            sql += 6;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "INSERT", 6) == 0)
        {
            token.type = TOKEN_INSERT;
            strcpy(token.token, "INSERT");
            sql += 6;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "CREATE", 6) == 0)
        {
            token.type = TOKEN_CREATE;
            strcpy(token.token, "CREATE");
            sql += 6;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "TABLES", 6) == 0)
        {
            token.type = TOKEN_TABLES;
            strcpy(token.token, "TABLES");
            sql += 6;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "DROP", 4) == 0)
        {
            token.type = TOKEN_DROP;
            strcpy(token.token, "DROP");
            sql += 4;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "TABLE", 5) == 0)
        {
            token.type = TOKEN_TABLE;
            strcpy(token.token, "TABLE");
            sql += 5;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "LOAD", 4) == 0)
        {
            token.type = TOKEN_LOAD;
            strcpy(token.token, "LOAD");
            sql += 4;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "DATABASES", 9) == 0)
        {
            token.type = TOKEN_DATABASES;
            strcpy(token.token, "DATABASES");
            sql += 9;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "SHOW", 4) == 0)
        {
            token.type = TOKEN_SHOW;
            strcpy(token.token, "SHOW");
            sql += 4;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "DATABASE", 8) == 0)
        {
            token.type = TOKEN_DATABASE;
            strcpy(token.token, "DATABASE");
            sql += 8;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "VALUES", 6) == 0)
        {
            token.type = TOKEN_VALUES;
            strcpy(token.token, "VALUES");
            sql += 6;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "UPDATE", 6) == 0)
        {
            token.type = TOKEN_UPDATE;
            strcpy(token.token, "UPDATE");
            sql += 6;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "DELETE", 6) == 0)
        {
            token.type = TOKEN_DELETE;
            strcpy(token.token, "DELETE");
            sql += 6;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "FROM", 4) == 0)
        {
            token.type = TOKEN_FROM;
            strcpy(token.token, "FROM");
            sql += 4;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "PRIMARY", 7) == 0)
        {
            token.type = TOKEN_PRIMARY;
            strcpy(token.token, "PRIMARY");
            sql += 7;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "KEY", 3) == 0)
        {
            token.type = TOKEN_KEY;
            strcpy(token.token, "KEY");
            sql += 3;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "char", 4) == 0)
        {
            token.type = TOKEN_CHAR;
            strcpy(token.token, "char");
            sql += 4;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "int", 3) == 0)
        {
            token.type = TOKEN_INT;
            strcpy(token.token, "int");
            sql += 3;
            tokens[token_count++] = token;
        }

        else if (strncmp(sql, "INTO", 4) == 0)
        {
            token.type = TOKEN_INTO;
            strcpy(token.token, "INTO");
            sql += 4;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "SET", 3) == 0)
        {
            token.type = TOKEN_SET;
            strcpy(token.token, "SET");
            sql += 3;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "WHERE", 5) == 0)
        {
            token.type = TOKEN_WHERE;
            strcpy(token.token, "WHERE");
            sql += 5;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, ">=", 2) == 0)
        {
            token.type = TOKEN_GE;
            strcpy(token.token, ">=");
            sql += 2;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "<=", 2) == 0)
        {
            token.type = TOKEN_LE;
            strcpy(token.token, "<=");
            sql += 2;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "<>", 2) == 0)
        {
            token.type = TOKEN_NE;
            strcpy(token.token, "<>");
            sql += 2;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "BETWEEN", 8) == 0)
        {
            token.type = TOKEN_BETWEEN;
            strcpy(token.token, "BETWEEN");
            sql += 8;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "LIKE", 4) == 0)
        {
            token.type = TOKEN_LIKE;
            strcpy(token.token, "LIKE");
            sql += 4;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "IN", 2) == 0)
        {
            token.type = TOKEN_IN;
            strcpy(token.token, "IN");
            sql += 2;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "AND", 3) == 0)
        {
            token.type = TOKEN_AND;
            strcpy(token.token, "AND");
            sql += 3;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "OR", 2) == 0)
        {
            token.type = TOKEN_OR;
            strcpy(token.token, "OR");
            sql += 2;
            tokens[token_count++] = token;
        }
        else if (strncmp(sql, "NOT", 3) == 0)
        {
            token.type = TOKEN_NOT;
            strcpy(token.token, "NOT");
            sql += 3;
            tokens[token_count++] = token;
        }
        else if (*sql == '+')
        {
            token.type = TOKEN_ADD;
            strcpy(token.token, "+");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == '-')
        {
            token.type = TOKEN_SUB;
            strcpy(token.token, "-");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == '/')
        {
            token.type = TOKEN_DIV;
            strcpy(token.token, "/");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == '=' && *(sql + 1) == '=')
        {
            token.type = TOKEN_EQ;
            strcpy(token.token, "==");
            sql += 2;
            tokens[token_count++] = token;
        }
        else if (*sql == '=')
        {
            token.type = TOKEN_EQ;
            strcpy(token.token, "=");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == ',')
        {
            token.type = TOKEN_COMMA;
            strcpy(token.token, ",");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == '.')
        {
            token.type = TOKEN_DOT;
            strcpy(token.token, ".");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == '>')
        {
            token.type = TOKEN_GT;
            strcpy(token.token, ">");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == '<')
        {
            token.type = TOKEN_LT;
            strcpy(token.token, "<");
            sql++;
            tokens[token_count++] = token;
        }

        else if (*sql == '*')
        {
            token.type = TOKEN_STAR;
            strcpy(token.token, "*");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == ';')
        {
            token.type = TOKEN_SEMICOLON;
            strcpy(token.token, ";");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == '(')
        {
            token.type = TOKEN_OPEN_PARENTHESIS;
            strcpy(token.token, "(");
            sql++;
            tokens[token_count++] = token;
        }
        else if (*sql == ')')
        {
            token.type = TOKEN_CLOSE_PARENTHESIS;
            strcpy(token.token, ")");
            sql++;
            tokens[token_count++] = token;
        }
        else if (isalpha(*sql))
        {
            int i = 0;
            while (isalnum(*sql) || *sql == '_')
            {
                token.token[i++] = *sql;
                sql++;
            }
            token.token[i] = '\0';
            token.type = TOKEN_IDENTIFIER;
            tokens[token_count++] = token;
        }
        else if (isdigit(*sql))
        {
            int i = 0;
            while (isdigit(*sql))
            {
                token.token[i++] = *sql;
                sql++;
            }
            token.token[i] = '\0';
            token.type = TOKEN_NUMBER;
            tokens[token_count++] = token;
        }
        else if (*sql == '\'')
        {
            sql++;
            int i = 0;
            while (*sql != '\'' && *sql != '\0')
            {
                token.token[i++] = *sql;
                sql++;
            }
            if (*sql == '\'')
            {
                token.token[i] = '\0';
                token.type = TOKEN_STRING;
                sql++;
                tokens[token_count++] = token;
            }
        }
    }
    *out_count = token_count;
    return tokens;
}

int parse_insert(Token *tokens, int token_count, int *iterator)
{
    if (!is_db_loaded())
    {
        printf("Error: No database is loaded, please load a database first\n");
        return -1;
    }
    // Check for the INTO keyword
    if (tokens[(*iterator)++].type != TOKEN_INTO)
    {
        printf("Error: Expected INTO keyword\n");
        return -1;
    }
    // Check for the identifier (table name)
    if (tokens[*iterator].type != TOKEN_IDENTIFIER)
    {
        printf("Error: Expected table name after INTO\n");
        return -1;
    }

    Table *target_table = get_table(tokens[*iterator].token);
    (*iterator)++;

    if (target_table == NULL)
    {
        printf("Error: Table %s does not exist\n", tokens[*iterator].token);
        return -1;
    }

    // check for VALUES
    if (tokens[(*iterator)++].type != TOKEN_VALUES)
    {
        printf("Error: Expected VALUES keyword\n");
        return -1;
    }

    // Check for the opening parenthesis
    if (tokens[(*iterator)++].type != TOKEN_OPEN_PARENTHESIS)
    {
        printf("Error: Expected opening parenthesis\n");
        return -1;
    }

    // get values
    void *values[target_table->columns_count];
    int *int_values = malloc(sizeof(int) * target_table->columns_count);
    char **string_values = malloc(sizeof(char *) * target_table->columns_count);
    if (!int_values || !string_values)
    {
        printf("Error: Memory allocation failed\n");
        free(int_values);
        free(string_values);
        return -1;
    }

    for (int i = 0; i < target_table->columns_count; i++)
    {
        if (token_count <= *iterator)
        {
            printf("Error: Unexpected end of tokens\n");
            return -1;
        }
        if (i > 0)
        {
            if (tokens[(*iterator)++].type != TOKEN_COMMA)
            {
                printf("Error: Expected comma\n");
                free(int_values);
                free(string_values);
                return -1;
            }
        }
        switch (target_table->columns[i].type)
        {
        case INT:
            if (tokens[*iterator].type != TOKEN_NUMBER)
            {
                printf("Error: Expected number for column %s\n", target_table->columns[i].name);
                free(int_values);
                free(string_values);
                return -1;
            }
            int_values[i] = atoi(tokens[(*iterator)++].token);
            values[i] = &int_values[i];
            break;

        case STRING:
            if (tokens[*iterator].type != TOKEN_STRING)
            {
                printf("Error: Expected string for column %s\n", target_table->columns[i].name);
                free(int_values);
                free(string_values);
                return -1;
            }
            string_values[i] = strdup(tokens[(*iterator)++].token);
            if (!string_values[i])
            {
                printf("Error: Memory allocation failed\n");
                free(int_values);
                free(string_values);
                return -1;
            }
            values[i] = string_values[i];
            break;
        }
    }

    // Check for the closing parenthesis
    if (tokens[(*iterator)++].type != TOKEN_CLOSE_PARENTHESIS)
    {
        printf("Error: Expected closing parenthesis\n");
        free(int_values);
        free(string_values);
        return -1;
    }
    // Check for the semicolon
    if (tokens[(*iterator)++].type != TOKEN_SEMICOLON)
    {
        printf("Error: Expected semicolon\n");
        free(int_values);
        free(string_values);
        return -1;
    }
    // Insert the record into the table
    if (insert_record_array(target_table, values) != 0)
    {
        printf("Error: Failed to insert record into table\n");
        free(int_values);
        free(string_values);
        return -1;
    }

    for (int i = 0; i < target_table->columns_count; i++)
    {
        if (target_table->columns[i].type == STRING && string_values[i])
        {
            free(string_values[i]);
        }
    }
    free(int_values);
    free(string_values);

    return 0;
}

int parse_where(Token *tokens, int token_count, int *iterator, Table *tables[], char *alias[], int table_count, long return_positions[][table_count], int *match_count)
{
    FILE *files[table_count];
    char *rows[table_count];
    for (int i = 0; i < table_count; i++)
    {
        files[i] = open_file(tables[i]->table_name, "bin", "rb");
        rows[i] = calloc(tables[i]->row_size_in_bytes, sizeof(char));
    }

    Expression *expr = parse_expression(tokens, iterator, token_count);
    if (tokens[*iterator].type == TOKEN_SEMICOLON)
    {
        nested_loop_join(expr, tables, alias, files, table_count, rows, 0, return_positions, match_count /*, columns, column_alias, column_count*/);
        if (match_count == 0)
        {
            printf("No matching records found\n");
            return -1;
        }
        else
        {
            for (int i = 0; i < table_count; i++)
            {
                free(rows[i]);
                fclose(files[i]);
            }
            return 0;
        }
    }
    else
    {
        printf("Error: Expected semicolon after WHERE clause\n");
        for (int i = 0; i < table_count; i++)
        {
            free(rows[i]);
            fclose(files[i]);
        }
        return -1;
    }
}

int parse_join(Token *tokens, int token_count, int *iterator, Table *tables[], char *alias[], int *table_count, int *total_record_size)
{
    while (1)
    {
        if (*iterator >= token_count)
        {
            printf("Error: Unexpected end of tokens\n");
            for (int i = 0; i < *table_count; i++)
            {
                free(alias[i]);
            }
            return -1;
        }
        if (tokens[*iterator].type != TOKEN_IDENTIFIER)
        {
            printf("Error: Expected table name after FROM or comma\n");
            for (int i = 0; i < *table_count; i++)
            {
                free(alias[i]);
            }
            return -1;
        }
        if (*table_count == MAX_JOIN_COUNT)
        {
            printf("Error: Exceeded max number of joins: %d\n", MAX_JOIN_COUNT);
            for (int i = 0; i < *table_count; i++)
            {
                free(alias[i]);
            }
            return -1;
        }
        tables[*table_count] = get_table(tokens[*iterator].token);

        if (tables[*table_count] == NULL)
        {
            printf("Error: Table %s does not exist\n", tokens[*iterator].token);
            for (int i = 0; i < *table_count; i++)
            {
                free(alias[i]);
            }
            return -1;
        }
        *total_record_size *= tables[*table_count]->row_size_in_bytes;
        (*iterator)++;
        if (tokens[*iterator].type == TOKEN_IDENTIFIER)
        {
            alias[*table_count] = strdup(tokens[*iterator].token);
            (*iterator)++;
        }
        else
        {
            alias[*table_count] = strdup(tables[*table_count]->table_name);
        }
        (*table_count)++;

        if (tokens[*iterator].type == TOKEN_COMMA)
        {
            (*iterator)++;
            continue;
        }
        else if (tokens[*iterator].type == TOKEN_WHERE || tokens[*iterator].type == TOKEN_EOF || tokens[*iterator].type == TOKEN_SEMICOLON)
        {
            break;
        }
        else
        {
            printf("Error: Invalid syntax on from, expected semicolon or WHERE\n");
            for (int i = 0; i < *table_count; i++)
            {
                free(alias[i]);
            }
            return -1;
        }
    }
    for (int i = 0; i < *table_count - 1; i++)
    {
        for (int j = i + 1; j < *table_count; j++)
        {
            if (strcmp(alias[i], alias[j]) == 0)
            {
                printf("Error: Alias %s used more than once\n", alias[i]);
                for (int i = 0; i < *table_count; i++)
                {
                    free(alias[i]);
                }
                return -1;
            }
        }
    }
    return 0;
}

int parse_select(Token *tokens, int token_count, int *iterator)
{
    if (!is_db_loaded())
    {
        printf("Error: No database is loaded, please load a database first\n");
        return -1;
    }
    int all = 0;
    char *column_names[MAX_COLUMN_COUNT];
    char *column_alias[MAX_COLUMN_COUNT];
    int column_count = 0;
    // Check for columns
    if (tokens[(*iterator)].type == TOKEN_STAR)
    {
        all = 1;
        (*iterator)++;
    }
    else if (tokens[*iterator].type == TOKEN_IDENTIFIER)
    {

        while (1)
        {
            if (tokens[*iterator].type == TOKEN_IDENTIFIER)
            {
                if (tokens[*iterator + 1].type == TOKEN_DOT)
                {
                    if (tokens[*iterator + 2].type != TOKEN_IDENTIFIER)
                    {
                        printf("Error: Invalid syntax on select\n");
                    }
                    else
                    {
                        column_alias[column_count] = strdup(tokens[*iterator].token);
                        (*iterator) += 2; // skip .
                        column_names[column_count] = strdup(tokens[*iterator].token);
                        (*iterator)++;
                    }
                }
                else
                {
                    column_alias[column_count] = strdup(tokens[*iterator].token);
                    column_names[column_count] = strdup(tokens[*iterator].token);
                    (*iterator)++;
                }
                column_count++;
                if (tokens[*iterator].type == TOKEN_COMMA)
                {
                    (*iterator)++;
                    continue;
                }
                else if (tokens[*iterator].type == TOKEN_FROM)
                {
                    break;
                }
                else
                {
                    printf("Error: Expected FROM or comma after column name\n");
                    for (int i = 0; i < column_count; i++)
                    {
                        free(column_names[i]);
                        free(column_alias[i]);
                    }
                    return -1;
                }
            }
            else
            {
                printf("Error: Expected column name\n");
                for (int i = 0; i < column_count; i++)
                {
                    free(column_names[i]);
                    free(column_alias[i]);
                }
                return -1;
            }
        }
        if (column_count == 0)
        {
            printf("Error: No columns specified\n If you want to select all columns, use '*'\n");
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(column_alias[i]);
            }
            return -1;
        }
    }
    else
    {
        printf("Error: Expected '*' or column name\n");
        return -1;
    }
    // Check for FROM keyword
    if (tokens[*iterator].type != TOKEN_FROM)
    {
        printf("Error: Expected FROM keyword\n");
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
            free(column_alias[i]);
        }
        return -1;
    }
    (*iterator)++;

    // Check for tables
    Table *tables[MAX_JOIN_COUNT];
    char *alias[MAX_JOIN_COUNT];
    int table_count = 0;
    int total_record_size = 1;

    if (parse_join(tokens, token_count, iterator, tables, alias, &table_count, &total_record_size) != 0)
    {
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
            free(column_alias[i]);
        }
        return -1;
    }

    for (int i = 0; i < table_count - 1; i++)
    {
        for (int j = i + 1; j < table_count; j++)
        {
            if (strcmp(alias[i], alias[j]) == 0)
            {
                printf("Error: Alias %s used more than once\n", alias[i]);
                for (int i = 0; i < column_count; i++)
                {
                    free(column_names[i]);
                    free(column_alias[i]);
                }
                for (int i = 0; i < table_count; i++)
                {
                    free(alias[i]);
                }
                return -1;
            }
        }
    }

    Column *columns[column_count];
    for (int i = 0; i < column_count; i++)
    {
        int check = 0;
        for (int j = 0; j < table_count; j++)
        {
            if (check_column_exists_by_name(tables[j], column_names[i]))
            {
                columns[i] = get_column(tables[j], column_names[i]);
                check++;
                if (check > 1)
                {
                    if (strcmp(column_alias[i], column_names[i]) == 0)
                    {
                        printf("Error: Column %s exists in more than one table, give specifications\n", column_names[i]);
                        for (int i = 0; i < column_count; i++)
                        {
                            free(column_names[i]);
                            free(column_alias[i]);
                            free(columns[i]);
                        }
                        for (int i = 0; i < table_count; i++)
                        {
                            free(alias[i]);
                        }
                        return -1;
                    }
                }
            }
        }
        if (!check)
        {
            printf("Error: Column %s does not exist in given tables\n", column_names[i]);
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(column_alias[i]);
                free(columns[i]);
            }
            for (int i = 0; i < table_count; i++)
            {
                free(alias[i]);
            }
            return -1;
        }
    }

    long return_positions[total_record_size][table_count];
    int match_count = 0;

    // // Check for WHERE keyword
    if (tokens[*iterator].type == TOKEN_WHERE)
    {
        (*iterator)++;
        parse_where(tokens, token_count, iterator, tables, alias, table_count, return_positions, &match_count);
        if (match_count == 0)
        {
            printf("No matching records found\n");
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(column_alias[i]);
            }
            for (int i = 0; i < table_count; i++)
            {
                free(alias[i]);
            }
            return 0;
        }
        // print positions
        for (int i = 0; i < match_count; i++)
        {
            for (int j = 0; j < table_count; j++)
            {
                printf("%ld, ", return_positions[i][j]);
            }
            printf("\n");
        }
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
            free(column_alias[i]);
        }
        for (int i = 0; i < table_count; i++)
        {
            free(alias[i]);
        }
        return 0; // when printing functions are ready this will be deleted

        if (all == 0)
        {
        }
        else
        {
            // print_all_tables_with_where(expr, tables, alias, table_count);
        }
    }

    // check semicolon
    else if (tokens[*iterator].type == TOKEN_SEMICOLON)
    {
        if (all == 0)
        {

            // print_joined_tables_without_where(tables, alias, files, table_count, columns, column_alias, column_count);
        }
        else
        {

            // print_all_tables_without_where(tables, alias, table_count);
        }
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
            free(column_alias[i]);
        }
        for (int i = 0; i < table_count; i++)
        {
            free(alias[i]);
        }
        return 0;
    }
    return -1;
}

int parse_create(Token *tokens, int token_count, int *iterator)
{
    if (tokens[*iterator].type == TOKEN_TABLE)
    {
        if (!is_db_loaded())
        {
            printf("Error: No database is loaded, please load a database first\n");
            return -1;
        }
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_IDENTIFIER)
        {
            printf("Error: Expected table name after CREATE TABLE\n");
            return -1;
        }
        char *table_name = strdup(tokens[*iterator].token);
        (*iterator)++;

        // check if table name already exists
        if (does_table_exists(table_name))
        {
            printf("Error: Table %s already exists\n", table_name);
            free(table_name);
            return -1;
        }
        if (tokens[*iterator].type != TOKEN_OPEN_PARENTHESIS)
        {
            printf("Error: Expected opening parenthesis after table name\n");
            free(table_name);
            return -1;
        }
        Column columns[MAX_COLUMN_COUNT];
        int column_count = 0;
        Column primary_key;
        int primary_key_found = 0;

        do
        {
            (*iterator)++;
            if (column_count >= MAX_COLUMN_COUNT)
            {
                printf("Error: Too many columns, Maximum is %d\n", MAX_COLUMN_COUNT);
                free(table_name);
                return -1;
            }
            if (*iterator >= token_count)
            {
                printf("Error: Unexpected end of tokens\n");
                free(table_name);
                return -1;
            }

            if (tokens[*iterator].type == TOKEN_IDENTIFIER)
            {
                strcpy(columns[column_count].name, tokens[*iterator].token);
                (*iterator)++;
                if (tokens[*iterator].type == TOKEN_INT)
                {
                    columns[column_count].type = INT;
                    columns[column_count].lenght = 0;
                    (*iterator)++;
                }
                else if (tokens[*iterator].type == TOKEN_CHAR)
                {
                    columns[column_count].type = STRING;
                    (*iterator)++;
                    if (tokens[*iterator].type != TOKEN_OPEN_PARENTHESIS || tokens[*(iterator) + 1].type != TOKEN_NUMBER)
                    {
                        printf("Error: Expected string lenght\n");
                        free(table_name);
                        return -1;
                    }
                    else
                    {
                        (*iterator)++;
                        columns[column_count].lenght = atoi(tokens[*iterator].token);
                        if (columns[column_count].lenght <= 0)
                        {
                            printf("Error: Invalid string lenght\n");
                            free(table_name);
                            return -1;
                        }
                        (*iterator)++;
                        if (tokens[*iterator].type != TOKEN_CLOSE_PARENTHESIS)
                        {
                            printf("Error: Expected closing parenthesis after string lenght\n");
                            free(table_name);
                            return -1;
                        }
                        (*iterator)++;
                    }
                }
                else
                {
                    printf("Error: Unknown column type %s\n", tokens[*iterator].token);
                    free(table_name);
                    return -1;
                }
                column_count++;
            }
            else if (tokens[*iterator].type == TOKEN_PRIMARY && tokens[*(iterator) + 1].type == TOKEN_KEY && tokens[*(iterator) + 2].type == TOKEN_OPEN_PARENTHESIS && primary_key_found == 0)
            {
                (*iterator) += 3;
                if (tokens[*iterator].type != TOKEN_IDENTIFIER)
                {
                    printf("Error: Expected primary key column name\n");
                    free(table_name);
                    return -1;
                }
                for (int i = 0; i < column_count; i++)
                {
                    if (strcmp(columns[i].name, tokens[*iterator].token) == 0)
                    {
                        strcpy(primary_key.name, columns[i].name);
                        primary_key.type = columns[i].type;
                        primary_key.lenght = columns[i].lenght;
                        (*iterator)++;
                        primary_key_found = 1;
                        break;
                    }
                }
                if (tokens[*iterator].type != TOKEN_CLOSE_PARENTHESIS)
                {
                    printf("Error: Expected closing parenthesis after primary key column name\n");
                    free(table_name);
                    return -1;
                }
                (*iterator)++;
            }
            else
            {
                printf("Error: Invalid syntax\n");
                free(table_name);
                return -1;
            }
        } while (tokens[*iterator].type == TOKEN_COMMA);
        if (tokens[*iterator].type != TOKEN_CLOSE_PARENTHESIS)
        {
            printf("Error: Expected closing parenthesis or comma\n");
            free(table_name);
            return -1;
        }
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon at the end of CREATE TABLE statement\n");
            free(table_name);
            return -1;
        }
        (*iterator)++;
        // Create the table
        Table *new_table = create_table(table_name, columns, column_count, primary_key);
        if (new_table == NULL)
        {
            printf("Error: Failed to create table\n");
            free(table_name);
            return -1;
        }
        // Add the table to the list of tables
        if (add_table_to_globals(new_table) != 0)
        {
            printf("Error: Failed to add table to globals\n");
            free(table_name);
            return -1;
        }
        if (update_table_count_on_file() != 0)
        {
            printf("Error: Failed to update table count on file\n");
            free(table_name);
            return -1;
        }
        free(table_name);
        return 0;
    }
    else if (tokens[*iterator].type == TOKEN_DATABASE)
    {
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_IDENTIFIER)
        {
            printf("Error: Expected database name after CREATE DATABASE\n");
            return -1;
        }
        char *db_name = strdup(tokens[*iterator].token);
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon");
        }
        (*iterator)++;
        // Create the database
        if (create_db(db_name) != 0)
        {
            printf("Error: Failed to create database\n");
            free(db_name);
            return -1;
        }
        free(db_name);
        return 0;
    }
    else
    {
        printf("Error: Expected TABLE or DATABASE keyword\n");
        return -1;
    }
}

int parse_delete(Token *tokens, int token_count, int *iterator)
{
    if (!is_db_loaded())
    {
        printf("Error: No database is loaded, please load a database first\n");
        return -1;
    }
    if (token_count <= *iterator)
    {
        printf("Error: Not enough tokens for DELETE statement\n");
        return -1;
    }
    if (tokens[*iterator].type != TOKEN_FROM)
    {
        printf("Error: Expected FROM keyword after DELETE\n");
        return -1;
    }
    (*iterator)++;
    if (tokens[*iterator].type != TOKEN_IDENTIFIER)
    {
        printf("Error: Expected table name after FROM\n");
        return -1;
    }
    char *table_name = strdup(tokens[*iterator].token);
    (*iterator)++;
    if (check_table_exist(table_name) == 0)
    {
        printf("Error: Table %s does not exist\n", table_name);
        free(table_name);
        return -1;
    }
    Table *target_table = get_table(table_name);
    if (target_table == NULL)
    {
        printf("Error: Table %s does not exist\n", table_name);
        free(table_name);
        return -1;
    }
    if (tokens[*iterator].type == TOKEN_FROM)
    {
        (*iterator)++;
        Table *tables[MAX_JOIN_COUNT];
        char *alias[MAX_JOIN_COUNT];
        int table_count = 0;
        int total_record_size = 1;
        int match_count = 0;

        if (parse_join(tokens, token_count, iterator, tables, alias, &table_count, &total_record_size) != 0)
        {
            free(table_name);
            return -1;
        }
        long positions[MAX_JOIN_COUNT][table_count];

        // Check for WHERE keyword
        if (tokens[*iterator].type != TOKEN_WHERE)
        {
            printf("Error: Expected WHERE keyword after table name\n");
            free(table_name);
            return -1;
        }
        (*iterator)++;
        if (parse_where(tokens, token_count, iterator, tables, alias, table_count, positions, &match_count) != 0)
        {
            free(table_name);
            return -1;
        }
        if (match_count == 0)
        {
            printf("No matching records found\n");
            free(table_name);
            return 0;
        }

        // semicolon check
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon after WHERE clause\n");
            free(table_name);
            return -1;
        }
        (*iterator)++;

        int target_index;
        for (int i = 0; i < table_count; i++)
        {
            if (target_table == tables[i])
            {
                target_index = i;
            }
        }

        for (int i = 0; i < match_count; i++)
        {
            long position = positions[i][target_index];
            if (isfree(target_table, position))
            {
                // printf("Record at position %ld is already deleted\n", position);
                continue;
            }
            if (delete_record_by_row_position(target_table, position) != 0)
            {
                printf("Error: Failed to delete record at position %ld\n", position);
                free(table_name);
                return -1;
            }
        }
        free(table_name);
        return 0;
    }
    else if (tokens[*iterator].type != TOKEN_WHERE)
    {
        printf("Error: Expected WHERE keyword after table name\n");
        free(table_name);
        return -1;
    }
    (*iterator)++;

    Table *tables[1];
    char *alias[1];
    tables[0] = target_table;
    alias[0] = strdup(target_table->table_name);
    int table_count = 1;
    long positions[target_table->record_size][1];
    int match_count = 0;

    if (parse_where(tokens, token_count, iterator, tables, alias, table_count, positions, &match_count) != 0)
    {
        free(table_name);
        return -1;
    }

    if (match_count == 0)
    {
        printf("No matching records found\n");
        free(table_name);
        return 0;
    }

    // semicolon check
    if (tokens[*iterator].type != TOKEN_SEMICOLON)
    {
        printf("Error: Expected semicolon after WHERE clause\n");
        free(table_name);
        return -1;
    }
    (*iterator)++;

    // Delete the records
    for (int i = 0; i < match_count; i++)
    {
        long position = positions[i][0];
        if (delete_record_by_row_position(target_table, position) != 0)
        {
            printf("Error: Failed to delete record at position %ld\n", position);
            free(table_name);
            return -1;
        }
    }
    free(table_name);
    free(alias[0]);
    return 0;
}

int parse_update(Token *tokens, int token_count, int *iterator)
{
    if (!is_db_loaded())
    {
        printf("Error: No database is loaded, please load a database first\n");
        return -1;
    }
    if (token_count <= *iterator)
    {
        printf("Error: Not enough tokens for UPDATE statement\n");
        return -1;
    }
    if (tokens[*iterator].type != TOKEN_IDENTIFIER)
    {
        printf("Error: Expected table name after UPDATE\n");
        return -1;
    }
    char *table_name = strdup(tokens[*iterator].token);
    (*iterator)++;
    if (check_table_exist(table_name) == 0)
    {
        printf("Error: Table %s does not exist\n", table_name);
        free(table_name);
        return -1;
    }
    Table *target_table = get_table(table_name);
    if (target_table == NULL)
    {
        printf("Error: Table %s does not exist\n", table_name);
        free(table_name);
        return -1;
    }
    if (tokens[*iterator].type != TOKEN_SET)
    {
        printf("Error: Expected SET keyword after table name\n");
        free(table_name);
        return -1;
    }
    (*iterator)++;
    char *column_names[MAX_COLUMN_COUNT];
    int column_count = 0;
    char *values[MAX_COLUMN_COUNT];
    while (1)
    {
        if (tokens[*iterator].type == TOKEN_IDENTIFIER)
        {
            column_names[column_count] = strdup(tokens[*iterator].token);
            (*iterator)++;
            if (column_count >= MAX_COLUMN_COUNT)
            {
                printf("Error: Too many columns, Maximum is %d\n", MAX_COLUMN_COUNT);
                free(table_name);
                for (int i = 0; i < column_count; i++)
                {
                    free(column_names[i]);
                    free(values[i]);
                }
                free(column_names[column_count]);
                return -1;
            }
            if (tokens[*iterator].type != TOKEN_EQ)
            {
                printf("Error: Expected '=' after column name\n");
                free(table_name);
                for (int i = 0; i < column_count; i++)
                {
                    free(column_names[i]);
                    free(values[i]);
                }
                free(column_names[column_count]);
                return -1;
            }

            (*iterator)++;
            if (tokens[*iterator].type != TOKEN_NUMBER && tokens[*iterator].type != TOKEN_STRING)
            {
                printf("Error: Expected value after column name\n");
                free(table_name);
                for (int i = 0; i < column_count; i++)
                {
                    free(column_names[i]);
                    free(values[i]);
                }
                free(column_names[column_count]);
                return -1;
            }
            (*iterator)++;
            values[column_count] = strdup(tokens[*iterator - 1].token);
            column_count++;
            if (tokens[*iterator].type == TOKEN_COMMA)
            {
                (*iterator)++;
                continue;
            }
            else if (tokens[*iterator].type == TOKEN_WHERE || tokens[*iterator].type == TOKEN_SEMICOLON)
            {
                break;
            }
            else
            {
                printf("Error: Expected comma or WHERE after value\n");
                free(table_name);
                for (int i = 0; i < column_count; i++)
                {
                    free(column_names[i]);
                    free(values[i]);
                }
                free(column_names[column_count]);
                return -1;
            }
        }
        else
        {
            printf("Error: Expected '=' after column name\n");
            free(table_name);
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(values[i]);
            }
            free(column_names[column_count]);
            return -1;
        }
    }

    if (tokens[*iterator].type == TOKEN_WHERE)
    {
        (*iterator)++;
        Table *tables[1];
        char *alias[1];
        tables[0] = target_table;
        alias[0] = strdup(target_table->table_name);
        int table_count = 1;
        int match_count = 0;

        long positions[target_table->record_size][table_count];

        if (parse_where(tokens, token_count, iterator, tables, alias, table_count, positions, &match_count) != 0)
        {
            printf("Error: Failed to parse WHERE clause\n");
            free(table_name);
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(values[i]);
            }
            return -1;
        }
        if (match_count == 0)
        {
            printf("No matching records found\n");
            free(table_name);
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(values[i]);
            }
            return 0;
        }

        // semicolon check
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon after WHERE clause\n");
            free(table_name);
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(values[i]);
            }
            return -1;
        }
        (*iterator)++;
        Column *columns[column_count];
        for (int j = 0; j < column_count; j++)
        {
            int check = 0;
            for (int k = 0; k < target_table->columns_count; k++)
            {
                if (strcmp(target_table->columns[k].name, column_names[j]) == 0)
                {
                    columns[j] = &target_table->columns[k];
                    check++;
                    break;
                }
            }
            if (check == 0)
            {
                printf("Error: Column %s does not exist in table %s\n", column_names[j], target_table->table_name);
                free(table_name);
                for (int i = 0; i < column_count; i++)
                {
                    free(column_names[i]);
                    free(values[i]);
                }
                return -1;
            }
        }
        FILE *file = open_file(target_table->table_name, "bin", "rb+");
        if (file == NULL)
        {
            printf("Error: Failed to open table file %s\n", target_table->table_name);
            free(table_name);
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
                free(values[i]);
            }
            return -1;
        }
        for (int i = 0; i < match_count; i++)
        {
            long position = positions[i][0];
            if (isfree(target_table, position))
            {
                // printf("Record at position %ld is already deleted\n", position);
                continue;
            }
            for (int j = 0; j < column_count; j++)
            {
                fseek(file, position + calculate_offset(target_table, *columns[j]), SEEK_SET);
                if (columns[j]->type == INT)
                {
                    int value = atoi(values[j]);
                    fwrite(&value, sizeof(int), 1, file);
                }
                else if (columns[j]->type == STRING)
                {
                    char *value = values[j];
                    fwrite(value, sizeof(char), columns[j]->lenght, file);
                }
                else
                {
                    printf("Error: Unknown column type %d\n", columns[j]->type);
                    fclose(file);
                    free(table_name);
                    for (int i = 0; i < column_count; i++)
                    {
                        free(column_names[i]);
                        free(values[i]);
                    }
                    return -1;
                }
            }
        }
        fclose(file);
        free(table_name);
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
            free(values[i]);
        }
        return 0;
    }
    else
    {
        printf("Error: Expected WHERE\n");
        free(table_name);
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
            free(values[i]);
        }
        return -1;
    }
}

int parse_show(Token *tokens, int token_count, int *iterator)
{
    if (token_count <= *iterator)
    {
        printf("Error: Not enough tokens for LOAD statement\n");
        return -1;
    }
    if (tokens[*iterator].type == TOKEN_DATABASES)
    {
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon");
        }
        (*iterator)++;
        list_dbs();
        return 0;
    }
    else if (tokens[*iterator].type == TOKEN_TABLES)
    {
        if (!is_db_loaded())
        {
            printf("Error: No database is loaded, please load a database first\n");
            return -1;
        }
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon");
        }
        (*iterator)++;
        list_tables();
        return 0;
    }
    return -1;
}

int parse_load(Token *tokens, int token_count, int *iterator)
{
    if (is_db_loaded())
    {
        printf("Error: A database is already loaded, please unload it first\n");
        return -1;
    }
    if (token_count <= *iterator)
    {
        printf("Error: Not enough tokens for LOAD statement\n");
        return -1;
    }
    if (tokens[*iterator].type != TOKEN_DATABASE)
    {
        printf("Error: Expected DATABASE keyword\n");
        return -1;
    }
    (*iterator)++;
    if (tokens[*iterator].type != TOKEN_IDENTIFIER)
    {
        printf("Error: Expected database name after LOAD DATABASE\n");
        return -1;
    }
    char *db_name = strdup(tokens[*iterator].token);
    (*iterator)++;
    if (tokens[*iterator].type != TOKEN_SEMICOLON)
    {
        printf("Error: Expected semicolon");
    }
    (*iterator)++;
    // Load the database

    if (load_db(db_name) != 0)
    {
        printf("Error: Failed to load database\n");
        free(db_name);
        return -1;
    }

    free(db_name);
    return 0;
}

int parse_drop(Token *tokens, int token_count, int *iterator)
{
    if (token_count <= *iterator)
    {
        printf("Error: Not enough tokens for DROP statement\n");
        return -1;
    }
    if (tokens[*iterator].type == TOKEN_TABLE)
    {
        if (!is_db_loaded())
        {
            printf("Error: No database is loaded, please load a database first\n");
            return -1;
        }
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_IDENTIFIER)
        {
            printf("Error: Expected table name after DROP TABLE\n");
            return -1;
        }
        char *table_name = strdup(tokens[*iterator].token);
        (*iterator)++;
        if (check_table_exist(table_name) == 0)
        {
            printf("Error: Table %s does not exist\n", table_name);
            free(table_name);
            return -1;
        }
        Table *table = get_table(table_name);
        if (table == NULL)
        {
            printf("Error: Table %s does not exist\n", table_name);
            free(table_name);
            return -1;
        }
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon\n");
            free(table_name);
            return -1;
        }
        (*iterator)++;

        // Drop the table
        if (drop_table(table) != 0)
        {
            printf("Error: Failed to drop table %s\n", table_name);
            free(table_name);
            return -1;
        }
        free(table_name);
        return 0;
    }
    else if (tokens[*iterator].type == TOKEN_DATABASE)
    {
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_IDENTIFIER)
        {
            printf("Error: Expected database name after DROP DATABASE\n");
            return -1;
        }
        char *db_name = strdup(tokens[*iterator].token);
        (*iterator)++;
        if (tokens[*iterator].type != TOKEN_SEMICOLON)
        {
            printf("Error: Expected semicolon");
        }
        (*iterator)++;
        // Drop the database
        if (drop_db(db_name) != 0)
        {
            printf("Error: Failed to drop database %s\n", db_name);
            free(db_name);
            return -1;
        }
        free(db_name);
        return 0;
    }
    else
    {
        printf("Error: Expected TABLE or DATABASE keyword\n");
        return -1;
    }
}

int parser(Token *tokens, int token_count)
{
    int iterator = 0;
    while (iterator < token_count)
    {
        switch (tokens[iterator].type)
        {
        case TOKEN_SELECT:
            iterator++;
            if (parse_select(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse SELECT statement\n");
                return -1;
            }
            break;
        case TOKEN_INSERT:
            iterator++;
            if (parse_insert(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse INSERT statement\n");
                return -1;
            }
            break;
        case TOKEN_SHOW:
            iterator++;
            if (parse_show(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse SHOW statement\n");
                return -1;
            }
            break;
        case TOKEN_LOAD:
            iterator++;
            if (parse_load(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse LOAD statement\n");
                return -1;
            }
            break;
        case TOKEN_CREATE:
            iterator++;
            if (parse_create(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse CREATE statement\n");
                return -1;
            }
            break;
        case TOKEN_DROP:
            iterator++;
            if (parse_drop(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse DROP statement\n");
                return -1;
            }
            break;
        case TOKEN_DELETE:
            iterator++;
            if (parse_delete(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse DROP statement\n");
                return -1;
            }
            break;
        case TOKEN_UPDATE:
            iterator++;
            if (parse_update(tokens, token_count, &iterator) == -1)
            {
                printf("Error: Failed to parse DROP statement\n");
                return -1;
            }
            break;
        case TOKEN_EOF:
            return 0;
        default:
            // Skip unhandled tokens (like semicolons, etc.)
            iterator++;
            break;
        }
    }
    return 0;
}