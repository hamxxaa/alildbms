#include <stdio.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "fnv_hash.h"

#define MAX_COLUMN_COUNT 20
#define MAX_NAME_LEN 50
#define DEFAULT_TABLE_SIZE 1000

typedef union
{
    int int_key;
    char *char_key;
} Key;

typedef enum
{
    INT,
    STRING
} DataType;

typedef struct HashEntry
{
    Key key;
    uint32_t hash;
    long file_pos;
    struct HashEntry *next;
} HashEntry;

// create a hash table struct
typedef struct
{
    HashEntry **buckets;
    int size;
    int entries;
} HashTable;

typedef struct
{
    char name[MAX_NAME_LEN];
    DataType type;
    int lenght; // for strings
} Column;

typedef struct
{
    char table_name[MAX_NAME_LEN];
    Column columns[MAX_COLUMN_COUNT];
    int columns_count;
    Column primary_key;
    char data_file[MAX_NAME_LEN + 5];
    int record_size;
    HashTable *hash;
    int row_size_in_bytes;
} Table;

HashTable *create_hashtable(int size)
{
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    ht->size = size;
    ht->buckets = (HashEntry **)calloc(size, sizeof(HashEntry *));
    return ht;
}

Table *create_table(char *table_name, Column *columns, int columns_count, Column primary_key)
{
    // create table
    Table *table = (Table *)malloc(sizeof(Table));
    if (!table)
    {
        perror("Failed to allocate table");
        return NULL;
    }

    // Copy columns
    for (int i = 0; i < columns_count; i++)
    {
        strncpy(table->columns[i].name, columns[i].name, MAX_NAME_LEN);
        table->columns[i].type = columns[i].type;
        table->columns[i].lenght = columns[i].lenght;
    }
    strncpy(table->table_name, table_name, MAX_NAME_LEN);
    table->record_size = 0;
    table->columns_count = columns_count;
    table->primary_key = primary_key;

    // open file
    char filename[MAX_NAME_LEN + 5];
    snprintf(filename, sizeof(filename), "%s.bin", table_name);
    strcpy(table->data_file, filename);
    FILE *file = fopen(filename, "wb+");
    if (!table->data_file)
    {
        perror("Failed to create data file");
        free(table);
        return NULL;
    }
    table->hash = create_hashtable(DEFAULT_TABLE_SIZE);

    // calculate row size in bytes
    int size = 0;
    for (int i = 0; i < columns_count; i++)
    {
        switch (columns[i].type)
        {
        case INT:
            size += 4;
            break;

        case STRING:
            size += columns[i].lenght + 1; // +1 for null terminator
            break;
        }
    }
    table->row_size_in_bytes = size;

    fclose(file);
    return table;
}

int insert_record(Table *table, ...)
{
    va_list args;
    va_start(args, table);

    // Open file in append mode
    FILE *file = fopen(table->data_file, "ab");
    if (!file)
    {
        perror("Failed to open data file");
        return -1;
    }

    Key key;
    uint32_t hash;
    long pos = ftell(file);
    // Write each column value based on its type
    for (int i = 0; i < table->columns_count; i++)
    {
        switch (table->columns[i].type)
        {
        case INT:
        {
            int val = va_arg(args, int);
            printf("val: %d\n", val);
            printf("pos: %ld\n", ftell(file));
            fwrite(&val, sizeof(int), 1, file);
            if (strcmp(table->columns[i].name, table->primary_key.name) == 0)
            {
                key.int_key = val;
                hash = fnv1a_hash_int(val);
            }
            break;
        }
        case STRING:
        {
            char *str = va_arg(args, char *);

            printf("str: %s\n", str);
            printf("pos: %ld\n", ftell(file));
            char *str_copy = calloc(1, table->columns[i].lenght + 1);
            if (str_copy == NULL)
            {
                perror("Memory allocation failed");
                fclose(file);
                va_end(args);
                return -1;
            }
            strncpy(str_copy, str, strlen(str));
            str_copy[table->columns[i].lenght] = '\0'; // Null-terminate the string
            // Write string length first?
            fwrite(str_copy, table->columns[i].lenght + 1, 1, file);
            if (table->columns[i].name == table->primary_key.name)
            {
                key.char_key = str_copy;
                hash = fnv1a_hash_str(str_copy);
            }
            break;
        }
        }
    }
    // create hash
    HashEntry *he = (HashEntry *)malloc(sizeof(HashEntry));
    he->file_pos = pos;
    printf("pos: %ld\n", pos);
    printf("hash: %u\n", hash);
    he->hash = hash;
    he->next = NULL;
    he->key = key;

    if (table->hash->buckets[hash % table->hash->size])
    {
        HashEntry *current = table->hash->buckets[hash % table->hash->size];
        while (current->next != NULL)
        {
            current = current->next;
        }
        table->hash->buckets[hash % table->hash->size]->next = he;
    }
    else
    {
        table->hash->buckets[hash % table->hash->size] = he;
    }

    table->record_size++;

    va_end(args);
    fclose(file);
    return 0;
}

char *search_record_by_key(Table *table, ...)
{
    va_list args;
    va_start(args, table);

    FILE *file = fopen(table->data_file, "rb");
    if (file == NULL)
    {
        perror("Failed to open data file");
        return NULL;
    }

    // calculate hash and index
    uint32_t hash;
    Key key;
    if (table->primary_key.type == INT)
    {
        key.int_key = va_arg(args, int);
        hash = fnv1a_hash_int(key.int_key);
    }
    if (table->primary_key.type == STRING)
    {
        key.char_key = va_arg(args, char *);
        hash = fnv1a_hash_str(key.char_key);
    }
    int index = hash % table->hash->size;

    HashEntry *he = table->hash->buckets[index];

    // find right entry in the bucket
    long pos;
    while (he)
    {
        if (he->key.int_key == key.int_key || he->key.char_key == key.char_key)
        {
            pos = he->file_pos;
            printf("Found at pos: %ld\n", pos);
            printf("Found at hash: %u\n", he->hash);
            break;
        }
        else
        {
            he = he->next;
        }
    }

    // read data
    fseek(file, pos, SEEK_SET);
    char *buffer = (char *)malloc(table->row_size_in_bytes);
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        free(buffer);
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread(buffer, 1, table->row_size_in_bytes, file);
    printf("bytes_read: %zu\n", bytes_read);
    printf("row_size_in_bytes: %d\n", table->row_size_in_bytes);
    if (bytes_read != table->row_size_in_bytes)
    {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    va_end(args);
    // return the data
    return buffer;
}

void print_row_readable(Table table, char *data)
{
    char *column_name[MAX_NAME_LEN];
    int intdata;
    char *chardata;

    for (int i = 0; i < table.columns_count; i++)
    {
        printf("%s: ", table.columns[i].name);
        switch (table.columns[i].type)
        {
        case INT:
            intdata = *(int *)data;
            printf("%d\n", intdata);
            data += sizeof(int);
            break;

        case STRING:
            chardata = malloc((table.columns[i].lenght + 1) * sizeof(char));
            memcpy(chardata, data, table.columns[i].lenght + 1);
            printf("%s\n", chardata);
            free(chardata);
            data += table.columns[i].lenght + 1; // Move pointer to the next column
            break;
        }
    }
}

int main()
{
    // Define columns
    Column columns[4] = {
        {"id", INT, 0},
        {"name", STRING, 50},
        {"age", INT, 0},
        {"email", STRING, 50}};

    // Primary key
    Column primary_key = {"id", INT, 0};

    // Create table
    Table *user_table = create_table("users", columns, 4, primary_key);
    if (!user_table)
    {
        printf("Failed to create table\n");
        return 1;
    }

    // Insert records
    if (insert_record(user_table, 1, "Alice", 25, "mailll") != 0)
    {
        printf("Failed to insert record 1\n");
    }
    if (insert_record(user_table, 2, "Bob", 30, "maill") != 0)
    {
        printf("Failed to insert record 2\n");
    }
    if (insert_record(user_table, 3, "Charlie", 35, "ma") != 0)
    {
        printf("Failed to insert record 3\n");
    }

    // Search records
    char *record = search_record_by_key(user_table, 2);
    if (record)
    {
        printf("Found record:\n");
        print_row_readable(*user_table, record);
        free(record);
    }
    else
    {
        printf("Record not found\n");
    }

    // Test with string primary key (would need another table)
    // ...

    return 0;
}
