#include <globals.h>
#include <stdio.h>
#include "table.h"
#include "file_io.h"
#include <stdlib.h>
#include <string.h>

struct Globals
{
    char *root;      // Root directory for the database
    Table **tables;  // Array of pointers to Table structs
    int table_count; // Size of the array
    char *db_name;   // Name of the current
    int db_loaded;   // Flag to indicate if the database is loaded
};

Globals globalvars;

int initialize_globals(const char *db_name)
{
    globalvars.tables = (Table **)malloc(sizeof(Table *) * MAX_TABLE_COUNT);
    if (!globalvars.tables)
    {
        printf("Failed to allocate memory for tables");
        return -1;
    }
    globalvars.table_count = 0;
    update_root(db_name);
    if (set_db_name(db_name) == -1)
    {
        printf("Failed to set database name\n");
        free(globalvars.tables);
        return -1;
    }
    for (int i = 0; i < MAX_TABLE_COUNT; i++)
    {
        globalvars.tables[i] = NULL;
    }
    if (get_tables() == -1)
    {
        printf("Failed to load tables from database %s\n", db_name);
        return -1;
    }
    globalvars.db_loaded = 1; // Set the database loaded flag
    return 0;
}

void update_root(const char *root)
{
    if (strlen(root) > MAX_NAME_LEN)
    {
        printf("Root name is too long\n");
        return;
    }
    if (globalvars.root)
    {
        free(globalvars.root);
    }
    globalvars.root = (char *)malloc(strlen(root) + 23); // 23 for "databases/" prefix
    if (!globalvars.root)
    {
        printf("Failed to allocate memory for root\n");
        return;
    }
    char newroot[MAX_NAME_LEN + 60];
    snprintf(newroot, sizeof(newroot), "databases/%s", root);
    strcpy(globalvars.root, newroot);
}

void free_globals()
{
    if (globalvars.root)
    {
        free(globalvars.root);
        globalvars.root = NULL;
    }
    if (globalvars.db_name)
    {
        free(globalvars.db_name);
        globalvars.db_name = NULL;
    }

    if (globalvars.tables)
    {
        for (int i = 0; i < globalvars.table_count; i++)
        {
            if (globalvars.tables[i] != NULL)
            {
                free_hashtable(globalvars.tables[i]->hash);
                free(globalvars.tables[i]);
            }
        }
    }
    free(globalvars.tables);
    globalvars.tables = NULL;
    globalvars.table_count = 0;
    globalvars.db_loaded = 0; // Reset the database loaded flag
}

char *get_root()
{
    if (globalvars.root == NULL)
    {
        printf("Root is not set\n");
        return NULL;
    }
    if (strlen(globalvars.root) == 0)
    {
        printf("Root is empty\n");
        return NULL;
    }
    return globalvars.root;
}

int is_db_loaded()
{
    return globalvars.db_loaded;
}

int add_table(const char *table_name)
{
    globalvars.tables[globalvars.table_count++] = read_table_metadata(table_name);
    if (globalvars.tables[globalvars.table_count - 1] == NULL)
    {
        printf("Failed to read table metadata for %s\n", table_name);
        return -1;
    }
    return 1;
}

int get_table_count()
{
    return globalvars.table_count;
}

int set_table_count(int count)
{
    if (count < 0)
    {
        printf("Table count cannot be negative\n");
        return -1;
    }
    globalvars.table_count = count;
    return 0;
}

Table **get_global_tables()
{
    if (globalvars.tables == NULL)
    {
        printf("Tables are not initialized\n");
        return NULL;
    }
    return globalvars.tables;
}

int set_db_name(const char *db_name)
{
    if (globalvars.db_name)
    {
        free(globalvars.db_name);
    }
    globalvars.db_name = (char *)malloc(strlen(db_name) + 1);
    if (!globalvars.db_name)
    {
        printf("Failed to allocate memory for db_name\n");
        return -1;
    }
    strcpy(globalvars.db_name, db_name);
    return 0;
}

int check_table_exist(const char *table_name)
{
    for (int i = 0; i < globalvars.table_count; i++)
    {
        if (globalvars.tables[i] != NULL && strcmp(globalvars.tables[i]->table_name, table_name) == 0)
        {
            return 1; // Table exists
        }
    }
    return 0; // Table does not exist
}

int add_table_to_globals(Table *table)
{
    if (globalvars.table_count >= MAX_TABLE_COUNT)
    {
        printf("Maximum table count reached: %d\n", MAX_TABLE_COUNT);
        return -1;
    }
    globalvars.tables[globalvars.table_count++] = table;
    return 0;
}

int remove_table_from_globals(const char *table_name)
{
    for (int i = 0; i < globalvars.table_count; i++)
    {
        if (globalvars.tables[i] != NULL && strcmp(globalvars.tables[i]->table_name, table_name) == 0)
        {
            free(globalvars.tables[i]);
            globalvars.tables[i] = NULL; // Set the pointer to NULL
            // Shift remaining tables
            for (int j = i; j < globalvars.table_count - 1; j++)
            {
                globalvars.tables[j] = globalvars.tables[j + 1];
            }
            globalvars.table_count--;
            return 0; // Table removed successfully
        }
    }
    printf("Table %s not found\n", table_name);
    return -1; // Table not found
}

Table *get_table(const char *table_name)
{
    for (int i = 0; i < globalvars.table_count; i++)
    {
        if (globalvars.tables[i] != NULL && strcmp(globalvars.tables[i]->table_name, table_name) == 0)
        {
            return globalvars.tables[i]; // Return the table if found
        }
    }
    printf("Table %s not found\n", table_name);
    return NULL; // Table not found
}