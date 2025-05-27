#ifndef GLOBALS_H
#define GLOBALS_H

#define DEFAULT_FREE_HASH_SPACES 500
#define MAX_COLUMN_COUNT 20
#define MAX_NAME_LEN 50
#define DEFAULT_TABLE_SIZE 1000
#define MAX_FREE_SPACES 500
#define MAX_TOKEN_LENGTH 256
#define MAX_TOKEN_COUNT 256
#define MAX_TABLE_COUNT 100
#define MAX_JOIN_COUNT 10

typedef struct Globals Globals;

extern Globals globalvars; // Array of pointers to Table structs

typedef struct Table Table; // Forward declaration of Table struct

/**
 * @brief Initialize global variables for the database.
 *
 * @param db_name The name of the database to initialize.
 * @return int 0 on success, -1 on failure.
 */
int initialize_globals();

/**
 * @brief Free all global variables and reset the state.
 */
void free_globals();

/**
 * @brief Update the root directory for the database.
 *
 * @param root The new root directory name.
 */
void update_root(const char *root);

/**
 * @brief Get the root directory for the database.
 *
 * @return char* Pointer to the root directory string, or NULL if not set.
 */
char *get_root();

/**
 * @brief Add a table to the global variables.
 *
 * @param table_name The name of the table to add.
 * @return int 1 on success, -1 on failure.
 */
int add_table(const char *table_name);

/**
 * @brief Get the current table count.
 *
 * @return int The number of tables currently loaded.
 */
int get_table_count();

/**
 * @brief Set the table count in the global variables.
 *
 * @param count The new table count to set.
 * @return int 0 on success, -1 on failure.
 */
int set_table_count(int count);

/**
 * @brief Get the global tables array.
 *
 * @return Table** Pointer to the array of Table pointers, or NULL if not initialized.
 */
Table **get_global_tables();

/**
 * @brief Check if a table exists in the global variables.
 *
 * @param table_name The name of the table to check.
 * @return int 1 if the table exists, 0 otherwise.
 */
int check_table_exist(const char *table_name);

/**
 * @brief Add a table to the global variables.
 *
 * @param table Pointer to the Table struct to add.
 * @return int 0 on success, -1 if maximum table count is reached.
 */
int add_table_to_globals(Table *table);

/**
 * @brief Get a table by its name from the global variables.
 *
 * @param table_name The name of the table to retrieve.
 * @return Table* Pointer to the Table struct if found, NULL otherwise.
 */
Table *get_table(const char *table_name);

/**
 * @brief Set the database name in the global variables.
 *
 * @param db_name The name of the database to set.
 * @return int 0 on success, -1 on failure.
 */
int set_db_name(const char *db_name);

/**
 * @brief Check if the database is loaded.
 *
 * @return int 1 if the database is loaded, 0 otherwise.
 */
int is_db_loaded();

/**
 * @brief Remove a table from the global variables and free its memory.
 *
 * @param table_name The name of the table to remove.
 * @return int 0 on success, -1 if the table is not found.
 */
int remove_table_from_globals(const char *table_name);

#endif // GLOBALS_H
