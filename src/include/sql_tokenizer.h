#ifndef SQL_TOKENIZER_H
#define SQL_TOKENIZER_H

#include "expression.h"
#include "table.h"
#include "globals.h"
#include <stdio.h>

typedef struct Expression Expression;
typedef enum
{
    TOKEN_SELECT,
    TOKEN_INSERT,
    TOKEN_CREATE,
    TOKEN_TABLE,
    TOKEN_DATABASE,
    TOKEN_SHOW,
    TOKEN_DOT,
    TOKEN_LOAD,
    TOKEN_DATABASES,
    TOKEN_TABLES,
    TOKEN_DROP,
    TOKEN_VALUES,
    TOKEN_UPDATE,
    TOKEN_DELETE,
    TOKEN_FROM,
    TOKEN_INTO,
    TOKEN_SET,
    TOKEN_PRIMARY,
    TOKEN_KEY,
    TOKEN_WHERE,
    TOKEN_IDENTIFIER,
    TOKEN_CHAR,
    TOKEN_INT,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_EQ,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_GE,
    TOKEN_LE,
    TOKEN_NE,
    TOKEN_BETWEEN,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_LIKE,
    TOKEN_IN,
    TOKEN_COMMA,
    TOKEN_STAR,
    TOKEN_SEMICOLON,
    TOKEN_OPEN_PARENTHESIS,
    TOKEN_CLOSE_PARENTHESIS,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_DIV,
    TOKEN_NOT,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct Token
{
    TokenType type;
    char token[MAX_TOKEN_LENGTH];
} Token;

/**
 * @brief Tokenize the given SQL string.
 *
 * @param sql The SQL string to tokenize.
 * @param out_count Pointer to store the number of tokens.
 * @return Token* Pointer to the array of tokens.
 */
Token *tokenize(const char *sql, int *out_count);

/**
 * @brief Parse the tokens and execute the corresponding SQL command by calling other parser functions.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @return int 0 on success, -1 on failure.
 */
int parser(Token *tokens, int token_count);

/**
 * @brief Parse an INSERT statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @return int 0 on success, -1 on failure.
 */
int parse_insert(Token *tokens, int token_count, int *iterator);

/**
 * @brief Parse a SELECT statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @return int 0 on success, -1 on failure.
 */
int parse_select(Token *tokens, int token_count, int *iterator);

/**
 * @brief Parse a CREATE statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 */
int parse_create(Token *tokens, int token_count, int *iterator);

/**
 * @brief Parse a SHOW statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @return int 0 on success, -1 on failure.
 */
int parse_show(Token *tokens, int token_count, int *iterator);

/**
 * @brief Parse a LOAD statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @return int 0 on success, -1 on failure.
 */
int parse_load(Token *tokens, int token_count, int *iterator);

/**
 * @brief Parse a DROP statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @return int 0 on success, -1 on failure.
 */
int parse_drop(Token *tokens, int token_count, int *iterator);

/**
 * @brief Parse a DELETE statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @return int 0 on success, -1 on failure.
 */
int parse_delete(Token *tokens, int token_count, int *iterator);

/**
 * @brief Parse an UPDATE statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @return int 0 on success, -1 on failure.
 */
int parse_update(Token *tokens, int token_count, int *iterator);

/**
 * @brief Get tables and aliases from a JOIN statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @param tables Pointer to an array of Table pointers to store the joined tables.
 * @param alias Pointer to an array of strings to store the aliases for the joined tables.
 * @param table_count Pointer to an integer to store the number of joined tables.
 * @param total_record_size Pointer to an integer to store the total record size of the joined tables.
 * @return int 0 on success, -1 on failure.
 */
int parse_join(Token *tokens, int token_count, int *iterator, Table **tables, char **alias, int *table_count, int *total_record_size);

/**
 * @brief Parse a WHERE clause in a SQL statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @param tables Pointer to an array of Table pointers to check conditions against.
 * @param alias Pointer to an array of strings for table aliases.
 * @param table_count The number of tables involved in the WHERE clause.
 * @param positions A 2D array to store positions of matching records.
 * @param match_count Pointer to an integer to store the number of matching records found.
 * @return int 0 on success, -1 on failure.
 */
int parse_where(Token *tokens, int token_count, int *iterator, Table **tables, char **alias, int table_count, long positions[][table_count], int *match_count);

/**
 * @brief Recursively perform a nested loop join on the given expression and tables to express joins and check using exression.
 * @param expr The expression to evaluate for the join.
 * @param tables The array of tables to join.
 * @param alias The array of aliases for the tables.
 * @param files The array of file pointers for the tables.
 * @param table_count The number of tables involved in the join.
 * @param rows The array of row data buffers for each table.
 * @param current_table_index The index of the current table being processed.
 * @param return_positions A 2D array to store the positions of matching records for each table.
 * @param match_count Pointer to an integer to count the number of matches found.
 * @param columns The array of columns for each table (optional, can be NULL).
 * @param column_alias The array of column aliases (optional, can be NULL).
 * @param column_count The number of columns in the tables (optional, can be 0).
 * @return void
 */
void nested_loop_join(Expression *expr, Table **tables, char **alias, FILE **files, int table_count, char **rows, int current_table_index, long return_positions[][table_count], int *match_count /*, Column **columns, char **column_alias, int column_count*/);
#endif // SQL_TOKENIZER_H
