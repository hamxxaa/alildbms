#include "sql_tokenizer.h"
#include "table.h"
#include "file_io.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

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
        else if (strncmp(sql, "TABLE", 5) == 0)
        {
            token.type = TOKEN_TABLE;
            strcpy(token.token, "TABLE");
            sql += 5;
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
        else if (strncmp(sql, "!=", 2) == 0)
        {
            token.type = TOKEN_NE;
            strcpy(token.token, "!=");
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

int parse_insert(Token *tokens, int token_count, int *iterator, Table **tables, int *table_count)
{
    if (token_count < 5)
    {
        printf("Error: Not enough tokens for INSERT statement\n");
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

    Table *target_table = NULL;
    // Check for the table name
    for (int i = 0; i < *table_count; i++)
    {
        if (strcmp(tables[i]->table_name, tokens[*iterator].token) == 0)
        {
            target_table = tables[i];
            (*iterator)++;
            break;
        }
    }
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

    for (int i = 0; i < tables[*table_count - 1]->columns_count; i++)
    {
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

    return 0;
}

int parse_select(Token *tokens, int token_count, int *iterator, Table **tables, int *table_count)
{
    if (token_count < 5)
    {
        printf("Error: Not enough tokens for SELECT statement\n");
        return -1;
    }

    int all = 0;
    char *column_names[MAX_COLUMN_COUNT];
    int column_count = 0;
    // Check for columns
    if (tokens[(*iterator)].type == TOKEN_STAR)
    {
        all = 1;
        (*iterator)++;
    }
    else if (tokens[*iterator].type == TOKEN_IDENTIFIER)
    {

        while (tokens[*iterator].type == TOKEN_IDENTIFIER)
        {
            column_names[column_count++] = strdup(tokens[*iterator].token);
            (*iterator)++;
            if (tokens[*iterator].type == TOKEN_COMMA)
            {
                (*iterator)++;
            }
            else
            {
                break;
            }
        }
        if (column_count == 0)
        {
            printf("Error: No columns specified\n If you want to select all columns, use '*'\n");
            for (int i = 0; i < column_count; i++)
            {
                free(column_names[i]);
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
        }
        return -1;
    }
    (*iterator)++;

    // Check for table name
    if (tokens[*iterator].type != TOKEN_IDENTIFIER)
    {
        printf("Error: Expected table name after FROM\n");
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
        }
        return -1;
    }
    Table *target_table = NULL;
    // Check for the table name

    for (int i = 0; i < *table_count; i++)
    {
        if (strcmp(tables[i]->table_name, tokens[*iterator].token) == 0)
        {
            target_table = tables[i];
            (*iterator)++;
            break;
        }
    }
    if (target_table == NULL)
    {
        printf("Error: Table %s does not exist\n", tokens[*iterator].token);
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
        }
        return -1;
    }
    Column *columns[column_count];

    if (all == 0)
    {
        // Check columns exist in given table
        for (int i = 0; i < column_count; i++)
        {
            columns[i] = check_column_exists_by_name(target_table, column_names[i]);
            if (columns[i] == NULL)
            {
                printf("Error: Column %s does not exist in table %s\n", column_names[i], target_table->table_name);
                free(column_names[i]);
                for (int j = 0; j < column_count; j++)
                {
                    free(column_names[j]);
                }
                return -1;
            }
        }
    }
    // Check for WHERE keyword
    if (tokens[*iterator].type == TOKEN_WHERE)
    {
    }
    // check semicolon
    else if (tokens[*iterator].type == TOKEN_SEMICOLON)
    {
        if (all)
        {
            print_all_columns(target_table);
        }
        else
        {
            print_values_of(target_table, columns, column_count);
        }
        for (int i = 0; i < column_count; i++)
        {
            free(columns[i]);
        }
        for (int i = 0; i < column_count; i++)
        {
            free(column_names[i]);
        }
        return 0;
    }
    return -1;
}

int parse_create(Token *tokens, int token_count, int *iterator, Table **tables, int *table_count)
{
    if (tokens[*iterator].type == TOKEN_TABLE)
    {
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
        tables[(*table_count)++] = new_table;
        free(table_name);
        return 0;
    }
    else
    {
        printf("Error: Expected TABLE keyword\n");
        return -1;
    }
}

int parser(Token *tokens, int token_count, Table **tables, int *table_count)
{
    int iterator = 0;
    for (int i = 0; i < token_count; i++)
    {
        switch (tokens[i].type)
        {
        case TOKEN_SELECT:
            iterator++;
            parse_select(tokens, token_count, &iterator, tables, table_count);
            break;
        case TOKEN_INSERT:
            // printf("INSERT statement\n");
            iterator++;
            if (parse_insert(tokens, token_count, &iterator, tables, table_count) == -1)
            {
                printf("Error: Failed to parse INSERT statement\n");
                return -1;
            }
            break;
        case TOKEN_UPDATE:
            // printf("UPDATE statement\n");
            break;
        case TOKEN_DELETE:
            // printf("DELETE statement\n");
            break;
        case TOKEN_CREATE:
            // printf("CREATE statement\n");
            iterator++;
            parse_create(tokens, token_count, &iterator, tables, table_count);
            break;
        case TOKEN_FROM:
            // printf("FROM clause\n");
            break;
        case TOKEN_INTO:
            // printf("INTO clause\n");
            break;
        case TOKEN_SET:
            // printf("SET clause\n");
            break;
        case TOKEN_WHERE:
            // printf("WHERE clause\n");
            break;
        case TOKEN_IDENTIFIER:
            // printf("Identifier: %s\n", tokens[i].token);
            break;
        case TOKEN_STRING:
            // printf("String: %s\n", tokens[i].token);
            break;
        case TOKEN_NUMBER:
            // printf("Number: %s\n", tokens[i].token);
            break;
        case TOKEN_EQ:
            // printf("Equal sign\n");
            break;
        case TOKEN_COMMA:
            // printf("Comma\n");
            break;
        case TOKEN_STAR:
            // printf("Asterisk\n");
            break;
        case TOKEN_SEMICOLON:
            // printf("Semicolon\n");
            break;
        default:
            // printf("Unknown token: %s\n", tokens[i].token);
        }
    }
    return 0;
}
