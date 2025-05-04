#ifndef FILE_IO_H
#define FILE_IO_H

#include "table.h"
#include "hashmap.h"
#include <stdio.h>

/**
 * @brief Open a file with the given name and mode.
 *
 * @param table_name The name of the table.
 * @param exit The exit path for the file.
 * @param mode The mode in which to open the file (e.g., "rb", "wb").
 * @return FILE* Pointer to the opened file.
 */
FILE *open_file(const char *table_name, const char *exit, const char *mode);

/**
 * @brief Create a hashmap file for the given table. It initializes the file with the size and number of entries.
 *
 * @param table The table for which the hashmap file is created.
 * @return int 0 on success, -1 on failure.
 */
int create_hashmap_file(Table *table);

/**
 * @brief Create a binary file for the given table.
 *
 * @param table The table for which the binary file is created.
 * @return int 0 on success, -1 on failure.
 */
int create_bin_file(Table *table);

/**
 * @brief Create a metadata file for the table and write tables metadata on the file.
 *
 * @param table The table whose metadata is to be stored.
 * @return int 0 on success, -1 on failure.
 */
int store_table_metadata(Table *table);

/**
 * @brief Update the record size indicator in the metadata file of the table.
 *
 * @param table The table whose record size is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_table_metadata_record_size(Table *table);

/**
 * @brief Update the number of entries indicator in the hashmap file for the given table.
 *
 * @param table The table whose hashmap file is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_hashmap_file_entries(Table *table);

/**
 * @brief Insert a hash entry into the hashmap file for the given table.
 *
 * @param table The table whose hashmap file is to be updated.
 * @param he The hash entry to be inserted.
 * @return int 0 on success, -1 on failure.
 */
int insert_to_hashmap_file(Table *table, const HashEntry *he);

/**
 * @brief Read the hashmap file for the given table and create a hash table from it.
 *
 * @param table The table whose hashmap file is to be read.
 * @return HashTable* Pointer to the hash table created from the file.
 */
HashTable *read_hashmap_file(Table *table);

/**
 * @brief Read the metadata of a table from the metadata file and create the table from the metadata.
 *
 * @param tablename The name of the table whose metadata is to be read.
 * @return Table* Pointer to the table structure containing the metadata.
 */
Table *read_table_metadata(const char *tablename);

#endif