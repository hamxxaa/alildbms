#include "table.h"
#include "file_io.h"
#include "fnv_hash.h"
#include "hashmap.h"
#include "sql_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>

Table *tables[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int table_count = 0;

void print_tableasd(Table *user_table)
{
    printf("Table name: %s\n", user_table->table_name);
    printf("Columns: ");
    for (int i = 0; i < user_table->columns_count; i++)
    {
        printf("%s ", user_table->columns[i].name);
    }
    printf("\n");
    printf("Columns count: %d\n", user_table->columns_count);
    printf("Primary key: %s\n", user_table->primary_key.name);
    printf("Record size: %d\n", user_table->record_size);
    printf("Row size in bytes: %d\n", user_table->row_size_in_bytes);
    printf("Free spaces: ");
    for (int i = 0; i < user_table->free_spaces_count; i++)
    {
        printf("%ld ", user_table->free_spaces[i]);
    }
    printf("\n");
    printf("Free spaces count: %d\n", user_table->free_spaces_count);

    printf("Hash table size: %d\n", user_table->hash->size);
    printf("Hash table entries: %d\n", user_table->hash->entries);
    printf("Hash table free spaces count: %d\n", user_table->hash->free_hash_spaces_count);
    printf("Hash table free spaces: ");
    for (int i = 0; i < user_table->hash->free_hash_spaces_count; i++)
    {
        printf("%ld ", user_table->hash->free_hash_spaces[i]);
    }
    printf("\n");
    printf("Hash table buckets positions:\n");
    for (int i = 0; i < user_table->hash->size; i++)
    {
        HashEntry *entry = user_table->hash->buckets[i];
        while (entry)
        {
            if (entry->file_pos != 0)
            {
                printf("Bucket:%d, %ld ", i, entry->file_pos);
                printf("\n");
            }

            entry = entry->next;
        }
    }
}

void other_print(Table *user_table)
{
    FILE *file = open_file(user_table->table_name, "hashmap", "rb");
    fseek(file, sizeof(int) * 2, SEEK_SET);
    int fsc;
    fread(&fsc, sizeof(int), 1, file);
    printf("free spaces count9: %d\n", fsc);
    fseek(file, sizeof(int) * 3 + sizeof(long) * (user_table->hash->free_hash_spaces_count - 1), SEEK_SET);
    printf("free spaces array: ");
    long a;
    fread(&a, sizeof(long), 1, file);
    printf("%ld ", a);
    printf("\n");
    printf("same from the struct:");
    for (int i = 0; i < user_table->hash->free_hash_spaces_count; i++)
    {
        printf("%ld ", user_table->hash->free_hash_spaces[i]);
    }
    printf("\n");
}

void insert(Table *user_table)
{
    // if (insert_record(user_table, 1, "Alice", 25, "mailll") != 0)
    // {
    //     printf("Failed to insert record 1\n");
    // }
    if (insert_record(user_table, 2, "Bob", 30, "maill") != 0)
    {
        printf("Failed to insert record 2\n");
    }
    if (insert_record(user_table, 3, "Charlie", 35, "ma") != 0)
    {
        printf("Failed to insert record 3\n");
    }
    if (insert_record(user_table, 4, "Charlie", 35, "mai") != 0)
    {
        printf("Failed to insert record 4\n");
    }
    if (insert_record(user_table, 5, "David", 40, "mail") != 0)
    {
        printf("Failed to insert record 5\n");
    }
    if (insert_record(user_table, 6, "Eve", 28, "mail") != 0)
    {
        printf("Failed to insert record 6\n");
    }
    if (insert_record(user_table, 7, "Frank", 22, "mail") != 0)
    {
        printf("Failed to insert record 7\n");
    }
    if (insert_record(user_table, 8, "Grace", 29, "mail") != 0)
    {
        printf("Failed to insert record 8\n");
    }
}

void search_and_print(Table *user_table)
{
    for (int i = 1; i < 8; i++)
    {
        char *record = search_record_by_key(user_table, i);
        if (record)
        {
            print_row_readable(user_table, record);
            free(record);
        }
        else
        {
            printf("Record not found\n");
        }
    }
}

Table *load()
{
    Table *user_table = read_table_metadata("users");
    if (!user_table)
    {
        printf("Failed to read table metadata\n");
        return NULL;
    }
    return user_table;
}

Table *create()
{
    Column columns[4] = {
        {"id", INT, 0},
        {"name", STRING, 50},
        {"age", INT, 0},
        {"email", STRING, 50}};
    Column primary_key = {"id", INT, 0};

    Table *user_table = create_table("users", columns, 4, primary_key);
    if (!user_table)
    {
        printf("Failed to create table\n");
        return NULL;
    }
    tables[table_count++] = user_table;
    return user_table;
}

void deletetest(Table *user_table)
{
    delete_record(user_table, 1);
    char *record = search_record_by_key(user_table, 1);
    if (record)
    {
        printf("Record not deleted\n");
        free(record);
    }
    else
    {
        printf("Record deleted successfully\n");
    }
}

void create_test()
{
    Table *user_table = create();
    if (!user_table)
    {
        printf("Failed to create table\n");
        return;
    };
    insert(user_table);
}

int main()
{
    create_test();
    char *query = "SELECT * FROM users;";
    printf("Query: %s\n", query);
    int token_count = 0;
    Token *tokens = tokenize(query, &token_count);
    if (parser(tokens, token_count, tables, table_count) == -1)
    {
        printf("Failed to parse query\n");
        free(tokens);
        return -1;
    }

    char *query2 = "INSERT INTO users VALUES (1, 'Alice', 25, 'mail');";
    printf("Query: %s\n", query2);
    int token_count2 = 0;
    Token *tokens2 = tokenize(query2, &token_count2);
    if (parser(tokens2, token_count2, tables, table_count) == -1)
    {
        printf("Failed to parse query\n");
        free(tokens2);
        return -1;
    }

    char *query3 = "SELECT * FROM users;";
    printf("Query: %s\n", query3);
    int token_count3 = 0;
    Token *tokens3 = tokenize(query3, &token_count3);
    // for (int i = 0; i < token_count; i++)
    // {
    //     printf("Token %d: Type: %d, Value: %s\n", i, tokens[i].type, tokens[i].token);
    // }
    if (parser(tokens3, token_count3, tables, table_count) == -1)
    {
        printf("Failed to parse query\n");
        free(tokens3);
        return -1;
    }
}
