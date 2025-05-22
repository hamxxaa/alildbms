#ifndef SQL_TOKENIZER_H
#define SQL_TOKENIZER_H

#include "table.h"

#define MAX_TOKEN_LENGTH 256
#define MAX_TOKEN_COUNT 256
typedef enum
{
    TOKEN_SELECT,
    TOKEN_INSERT,
    TOKEN_CREATE,
    TOKEN_TABLE,
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
 * @param tables The array of tables to operate on.
 * @param table_count Pointer to the number of tables.
 * @return int 0 on success, -1 on failure.
 */
int parser(Token *tokens, int token_count, Table **tables, int *table_count);

/**
 * @brief Parse an INSERT statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @param tables The array of tables to operate on.
 * @param table_count Pointer to the number of tables.
 * @return int 0 on success, -1 on failure.
 */
int parse_insert(Token *tokens, int token_count, int *iterator, Table **tables, int *table_count);

/**
 * @brief Parse a SELECT statement.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @param tables The array of tables to operate on.
 * @param table_count Pointer to the number of tables.
 * @return int 0 on success, -1 on failure.
 */
int parse_select(Token *tokens, int token_count, int *iterator, Table **tables, int *table_count);

/**
 * @brief Parse a WHERE clause.
 *
 * @param tokens The array of tokens to parse.
 * @param token_count The number of tokens.
 * @param iterator Pointer to the current position in the token array.
 * @param tables The array of tables to operate on.
 * @param table_count The number of tables.
 * @return int 0 on success, -1 on failure.
 */
int parse_where(Token *tokens, int token_count, int *iterator, Table **tables, int *table_count); // TO BE IMPLEMENTED

#endif // SQL_TOKENIZER_H