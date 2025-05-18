#ifndef TABLE_H
#define TABLE_H

#define MAX_COLUMN_COUNT 20
#define MAX_NAME_LEN 50
#define DEFAULT_TABLE_SIZE 1000
#define MAX_FREE_SPACES 500

#include "hashmap.h"
#include "fnv_hash.h"
#include <stdlib.h>
#include <stdarg.h>

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
    long free_spaces[MAX_FREE_SPACES];
    int free_spaces_count;
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
char *search_record_by_key(const Table *table, ...);

/**
 * @brief print the binary-form record in a human-readable format.
 *
 * @param table The table structure containing the metadata.
 * @param data The binary data of the record.
 * @return void
 */
void print_row_readable(const Table *table, const char *data);

/**
 * @brief Check if a column exists in the table.
 *
 * @param table The table to check in.
 * @param column The column to check for.
 * @return int 1 if the column exists, 0 otherwise.
 */
int check_column_exists(const Table *table, const Column column);

/**
 * @brief Check if a column exists in the table by its name.
 *
 * @param table The table to check in.
 * @param column_name The name of the column to check for.
 * @return Column The column if it exists, NULL otherwise.
 *        Note: The caller is responsible for freeing the returned column.
 */
Column *check_column_exists_by_name(const Table *table, const char *column_name);

/**
 * @brief Calculate the offset of a column in the table.
 *
 * @param table The table to check in.
 * @param column The column to calculate the offset for.
 * @return int The offset of the column, or -1 if not found.
 */
int calculate_offset(const Table *table, const Column column);

/**
 * @brief find the position of a record in the file it gets the primary key from table and takes the correct value from args given.
 *
 * @param table The table to search in.
 * @param args The variable arguments list containing the primary key value.
 * @return long The position of the record in the file, or -1 if table does not exist,-2 if primary key is invalid, -3 if hash entry could not be created.
 */
long find_record_position(const Table *table, va_list args);

/**
 * @brief Update a record in the table.
 *
 * @param table The table to update the record in.
 * @param column The column to update.
 * @param ... Primary key and The new value for the column.
 * @return int 0 on success, -1 on failure.
 */
int update_record(const Table *table, const Column column, ...);

/**
 * @brief Compare two columns by their names.
 *
 * @param a The first column.
 * @param b The second column.
 * @return int 0 if equal, non-zero otherwise.
 */
int cmpcolumns(const Column a, const Column b);

/**
 * @brief deletes a record from the table.
 *
 * @param table The table to delete the record from.
 * @param ... The primary key value to delete.
 * @return int 0 on success, -1 on failure.
 */
int delete_record(Table *table, ...);

/**
 * @brief Delete a table and free its memory.
 *
 * @param table The table to delete.
 * @return int 0 on success, -1 on failure.
 */
int free_table(Table *table);

/**
 * @brief Like insert_record but takes an array of values.
 *
 * @param table The table to insert the record into.
 * @param values The array of values to insert into the record.
 * @return int 0 on success, -1 on failure.
 */
int insert_record_array(Table *table, void **values);

/**
 * @brief Create a temporary table with the given name and columns.
 *
 * @param table_name The name of the temporary table.
 * @param columns The columns of the temporary table.
 * @param columns_count The number of columns in the temporary table.
 * @return Table* Pointer to the created temporary table.
 */
Table *create_temp_table(const char *table_name, const Column *columns, const int columns_count);

#endif