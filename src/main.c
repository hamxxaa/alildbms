#include "table.h"
#include "file_io.h"
#include "fnv_hash.h"
#include "hashmap.h"
#include "sql_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void get_query(char *query)
{
    printf("Query: %s\n", query);
    int token_count = 0;
    Token *tokens = tokenize(query, &token_count);
    if (parser(tokens, token_count) == -1)
    {
        printf("Failed to parse query\n");
    }
    free(tokens);
}

void setup()
{
    get_query("DROP DATABASE test_db;");
    get_query("CREATE DATABASE test_db;");
    get_query("SHOW DATABASES;");
    get_query("LOAD DATABASE test_db;");
    get_query("CREATE TABLE users (ID int, name char(50), age int, email char(20), PRIMARY KEY(ID));");
    get_query("INSERT INTO users VALUES (1, 'ALICE', 25, 'mail');");
    get_query("INSERT INTO users VALUES (2, 'BOB', 30, 'mail');");
    get_query("INSERT INTO users VALUES (3, 'CHARLIE', 35, 'mail');");
    get_query("INSERT INTO users VALUES (4, 'DAVID', 40, 'mail');");
    get_query("INSERT INTO users VALUES (5, 'EVE', 28, 'mail');");
    get_query("INSERT INTO users VALUES (6, 'FRANK', 22, 'mail');");
    get_query("INSERT INTO users VALUES (7, 'GRACE', 29, 'mail');");
    get_query("SELECT * FROM users WHERE ID = 1;");
    get_query("CREATE TABLE products (ID int, name char(50), price int, PRIMARY KEY(ID));");
    get_query("INSERT INTO products VALUES (1, 'Product1', 100);");
    get_query("INSERT INTO products VALUES (2, 'Product2', 200);");
    get_query("SELECT * FROM products;");
    get_query("SHOW TABLES;");
    get_query("DROP TABLE users;");
    get_query("SHOW TABLES;");
    get_query("CREATE TABLE use (ID int, PRIMARY KEY(ID));");
    get_query("INSERT INTO use VALUES (1);");
    get_query("INSERT INTO use VALUES (2);");
    get_query("SELECT * FROM use;");
    get_query("SHOW TABLES;");
    get_query("DROP TABLE use;");
    get_query("SHOW TABLES;");
    get_query("DROP TABLE products;");
}

int main()
{
    // Initialization (optional for testing)
    // printf("AlilDBMS listening for queries...\n");

    // setup(); // Uncomment to run the setup queries

    char query[2048]; // Enough for a single SQL line

    while (fgets(query, sizeof(query), stdin))
    {
        // Remove trailing newline (for compatibility)
        query[strcspn(query, "\n")] = 0;

        // Skip empty inputs
        if (strlen(query) == 0)
            continue;

        // Tokenize and parse
        int token_count = 0;
        Token *tokens = tokenize(query, &token_count);
        if (!tokens)
        {
            printf("!END!\n");
            fflush(stdout);
            continue;
        }

        if (parser(tokens, token_count) == -1)
        {
            printf("Error: Could not parse query.\n");
        }

        free(tokens);

        // Notify end of result block
        printf("!END!\n");
        fflush(stdout);
    }

    return 0;
}
