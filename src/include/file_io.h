#ifndef FILE_IO_H
#define FILE_IO_H

#include "table.h"
#include "hashmap.h"
#include "globals.h"
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
int create_hashmap_file(const Table *table);

/**
 * @brief Create a binary file for the given table.
 *
 * @param table The table for which the binary file is created.
 * @return int 0 on success, -1 on failure.
 */
int create_bin_file(const Table *table);

/**
 * @brief Create a metadata file for the table and write tables metadata on the file.
 *
 * @param table The table whose metadata is to be stored.
 * @return int 0 on success, -1 on failure.
 */
int store_table_metadata(const Table *table);

/**
 * @brief Update the record size indicator in the metadata file of the table.
 *
 * @param table The table whose record size is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_table_metadata_record_size(const Table *table);

/**
 * @brief Update the number of entries indicator in the hashmap file for the given table.
 *
 * @param table The table whose hashmap file is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_hashmap_file_entries(const Table *table);

/**
 * @brief Insert a hash entry into the hashmap file for the given table.
 *
 * @param table The table whose hashmap file is to be updated.
 * @param he The hash entry to be inserted.
 * @return int 0 on success, -1 on failure.
 */
int insert_to_hashmap_file(const Table *table, HashEntry *he);

/**
 * @brief Read the hashmap file for the given table and create a hash table from it.
 *
 * @param table The table whose hashmap file is to be read.
 * @return HashTable* Pointer to the hash table created from the file.
 */
HashTable *read_hashmap_file(const Table *table);

/**
 * @brief Write a string to a file.
 *
 * @param file The file to write to.
 * @param val The string value to write.
 * @param length The length of the string.
 * @return int 0 on success, -1 on failure.
 */
int write_string_to_file(FILE *file, const char *val, int length);

/**
 * @brief Read the metadata of a table from the metadata file and create the table from the metadata.
 *
 * @param tablename The name of the table whose metadata is to be read.
 * @return Table* Pointer to the table structure containing the metadata.
 */
Table *read_table_metadata(const char *tablename);

/**

 * @brief Update the metadata file of the table with the new free spaces count.
 *
 * @param table The table whose metadata file is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_table_metadata_free_spaces(const Table *table);

/**
 * @brief Update the metadata file of the table with the new free spaces count.
 *
 * @param table The table whose metadata file is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_table_metadata_free_spaces_count(const Table *table);

/**
 * @brief Delete entry from the hashmap file.
 *
 * @param table The table to delete the entry from.
 * @param he The hash entry to delete.
 * @return int 0 on success, -1 on failure.
 */
int delete_entry_from_hashmap_file(const Table *table, const HashEntry *he);

/**
 * @brief Delete a record from the binary file of the table.
 *
 * @param table The table from which to delete the record.
 * @param pos The position of the record in the file.
 * @return int 0 on success, -1 on failure.
 */
int delete_record_from_file(const Table *table, long pos);

/**
 * @brief Update the free spaces count in the hashmap file.
 *
 * @param table The table whose hashmap file is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_hashmap_file_free_spaces_count(const Table *table);

/**
 * @brief Update the free spaces in the hashmap file.
 *
 * @param table The table whose hashmap file is to be updated.
 * @return int 0 on success, -1 on failure.
 */
int update_hashmap_file_free_spaces(const Table *table);

/**
 * @brief Read all data from the binary file of the table.
 *
 * @param table The table whose binary file is to be read.
 * @return char* Pointer to the read data, or NULL on failure.
 */
char *read_all_bin_file(const Table *table);

/**
 * @brief Print the values of the specified columns in the table.
 *
 * @param table The table to print the values from.
 * @param columns The columns to print.
 * @param columns_count The number of columns to print.
 */
void print_values_of(const Table *table, Column **columns, int columns_count);

/**
 * @brief Print all columns of the table.
 *
 * @param table The table to print the columns of.
 */
void print_all_columns(const Table *table);

/**
 * @brief Add a new table to the .tables file.
 */
int add_table_to_tables(const char *table_name);

/**
 * @brief Remove a table from the .tables file.
 *
 * @param table_name The name of the table to remove.
 * @return int 0 on success, -1 on failure.
 */
int remove_table_from_tables(const char *table_name);

/**
 * @brief Check if a table exists in the .tables file.
 *
 * @param table_name The name of the table to check.
 * @return int 1 if the table exists, 0 if it does not, -1 on error.
 */
int does_table_exists(const char *table_name);

/**
 * @brief Create a new database with the specified name.
 *
 * @param db_name The name of the database to create.
 * @return int 0 on success, -1 on failure.
 */
int create_db(const char *db_name);

/**
 * @brief Create a directory with the specified npath.
 *
 * @param path The path of the directory to create.
 * @return int 0 on success, -1 on failure.
 */
int create_directory(const char *path);

/**
 * @brief Create initial folders for the database.
 *
 * @param db_name The name of the database for which to create initial folders.
 * @return int 0 on success, -1 on failure.
 */
int create_db_initial_folders(const char *db_name);

/**
 * @brief Check if a database exists.
 * 
 * @param db_name The name of the database to check.
 * @return int 1 if the database exists, 0 if it does not.
 */
int check_db_exists(const char *db_name);

/**
 * @brief Print the list of databases.
 * 
 * @return int 0 on success, -1 on failure.
 */
int list_dbs();

/**
 * @brief Drop a database, delete its files and free allocated memory.
 *  
 * @param db_name The name of the database to drop.
 * @return int 0 on success, -1 on failure.
 */
int drop_db(const char *db_name);

/**
 * @brief Remove a directory and all its contents recursively.
 * 
 * @param path The path of the directory to remove.
 * @return int 0 on success, -1 on failure.
 */
int remove_directory(const char *path);

/**
 * @brief Load a database by its name.
 * 
 * @param db_name The name of the database to load.
 * @return int 0 on success, -1 on failure.
 */
int load_db(const char *db_name);

/**
 * @brief Create the .tables file for the database and initialize table_count.
 * 
 * @param db_name The name of the database for which to create the .tables file.
 * @return int 0 on success, -1 on failure.
 */
int create_tables_file(const char *db_name);

/**
 * @brief Update the table count in the .tables file with value from globalvars.
 * 
 * @return int 0 on success, -1 on failure.
 */
int update_table_count_on_file();

/**
 * @brief List all tables in the current database by reading the .tables file.
 * 
 * @return int 0 on success, -1 on failure.
 */
int list_tables();

/**
 * @brief Read every table in the .tables file and create the tables.
 * 
 * @return int 0 on success, -1 on failure.
 */
int get_tables();

#endif