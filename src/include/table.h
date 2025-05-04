#ifndef TABLE_H
#define TABLE_H

#define MAX_COLUMN_COUNT 20
#define MAX_NAME_LEN 50
#define DEFAULT_TABLE_SIZE 1000

#include "hashmap.h"
#include "fnv_hash.h"
#include <stdlib.h>

typedef enum
{
    INT,
    STRING
} DataType;

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
    int record_size;
    HashTable *hash;
    int row_size_in_bytes;
} Table;

/**
 * @brief Calculate the size of a row in bytes based on the columns.
 *
 * @param columns The columns of the table.
 * @param columns_count The number of columns in the table.
 * @return int The size of a row in bytes.
 */
int calculate_row_size_in_bytes(const Column *columns, const int columns_count);

/**
 * @brief Create initial files for the table; including binary, hashmap and metadata files.
 *
 * @param table The table for which to create the initial files.
 * @return int 0 on success, -1 on failure.
 */
int create_initial_files_for_table(Table *table);

/**
 * @brief Create a table with the given name, columns, and primary key.
 *
 * @param table_name The name of the table.
 * @param columns The columns of the table.
 * @param columns_count The number of columns in the table.
 * @param primary_key The primary key of the table.
 * @return Table* Pointer to the created table.
 */
Table *create_table(const char *table_name, const Column *columns, const int columns_count, const Column primary_key);

/**
 * @brief Insert a record into the table.
 *
 * @param table The table to insert the record into.
 * @param ... The values to insert into the record.
 * @return int 0 on success, -1 on failure.
 */
int insert_record(Table *table, ...);

/**
 * @brief Search for a record in the table by its primary key.
 *
 * @param table The table to search in.
 * @param ... The primary key value to search for.
 * @return char* Pointer to the record binary data, or NULL if not found.
 */
char *search_record_by_key(Table *table, ...);

/**
 * @brief print the binary-form record in a human-readable format.
 *
 * @param table The table structure containing the metadata.
 * @param data The binary data of the record.
 * @return void
 */
void print_row_readable(Table table, const char *data);

#endif