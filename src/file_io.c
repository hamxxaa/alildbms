#include "file_io.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

FILE *open_file(const char *table_name, const char *exit, const char *mode)
{
    char filename[MAX_NAME_LEN * 2 + 23];
    snprintf(filename, sizeof(filename), "%s/%ss/%s.%s", get_root(), exit, table_name, exit);
    FILE *file = fopen(filename, mode);
    if (!file)
    {
        perror("Failed to create data file");
        return NULL;
    }

    return file;
}

int list_tables()
{
    char path[MAX_NAME_LEN + 8];
    snprintf(path, sizeof(path), "%s/.tables", get_root());
    FILE *file = fopen(path, "rb");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }

    int table_count;
    fread(&table_count, sizeof(int), 1, file);
    if (table_count <= 0)
    {
        printf("No tables found\n");
        fclose(file);
        return 0;
    }
    char table_name[MAX_NAME_LEN + 1];
    for (int i = 0; i < table_count; i++)
    {
        fread(table_name, sizeof(char), MAX_NAME_LEN + 1, file);
        if (strlen(table_name) > 0)
        {
            printf("%s\n", table_name);
        }
    }
    fclose(file);
    return 0;
}

int create_directory(const char *dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1)
    {
        if (mkdir(dir, 0755) == -1)
        {
            perror("Failed to create directory");
            return -1;
        }
    }
    return 0;
}

int create_db_initial_folders(const char *db_name)
{
    char dir[MAX_NAME_LEN + 20];
    snprintf(dir, sizeof(dir), "databases/%s", db_name);
    if (create_directory(dir) == -1)
    {
        return -1;
    }
    snprintf(dir, sizeof(dir), "databases/%s/bins", db_name);
    if (create_directory(dir) == -1)
    {
        return -1;
    }
    snprintf(dir, sizeof(dir), "databases/%s/hashmaps", db_name);
    if (create_directory(dir) == -1)
    {
        return -1;
    }
    snprintf(dir, sizeof(dir), "databases/%s/metadatas", db_name);
    if (create_directory(dir) == -1)
    {
        return -1;
    }
    snprintf(dir, sizeof(dir), "databases/%s/.tables", db_name);
    if (create_tables_file(dir) == -1)
    {
        return -1;
    }
    return 0;
}

int check_db_exists(const char *db_name)
{
    char db_path[MAX_NAME_LEN + 20];
    snprintf(db_path, sizeof(db_path), "databases/%s", db_name);
    struct stat st = {0};
    if (stat(db_path, &st) == 0)
    {
        return 1; // Directory exists
    }
    return 0; // Directory does not exist
}

int list_dbs()
{
    struct dirent *entry;
    DIR *dp = opendir("./databases");
    if (dp == NULL)
    {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_type == 4 && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) // for some reason DT_DIR does not exist, its 4 in enum so i used 4
        {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dp);
    return 0;
}

int load_db(const char *db_name)
{
    if (!check_db_exists(db_name))
    {
        printf("Database does not exist\n");
        return -1;
    }
    if (initialize_globals(db_name) == -1)
    {
        printf("Failed to initialize globals\n");
        return -1;
    }
    return 0;
}

int create_db(const char *db_name)
{
    if (check_db_exists(db_name))
    {
        printf("Database already exists\n");
        return -1;
    }
    if (create_db_initial_folders(db_name) == -1)
    {
        return -1;
    }
    return 0;
}

int get_tables()
{
    char root[MAX_NAME_LEN + 8];
    snprintf(root, sizeof(root), "%s/.tables", get_root());
    FILE *file = fopen(root, "rb+");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }
    int table_count;
    fread(&table_count, sizeof(int), 1, file);
    for (int i = 0; i < table_count; i++)
    {
        char table_name[MAX_NAME_LEN + 1];
        fread(table_name, sizeof(table_name), 1, file);
        if (add_table(table_name) == -1)
        {
            fclose(file);
            return -1;
        }
    }
    if (table_count != get_table_count())
    {
        printf("Error: Unexpected table or table count");
        return -1;
    }
    fflush(file);
    fclose(file);
    return 0;
}

int remove_directory(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        perror("Failed to open directory");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (lstat(full_path, &statbuf) == -1)
        {
            perror("Failed to get file status");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            remove_directory(full_path); // Recursive call for subdirectories
        }
        else
        {
            if (unlink(full_path) == -1)
            {
                perror("Failed to delete file");
                closedir(dir);
                return -1;
            }
        }
    }
    closedir(dir);

    if (rmdir(path) == -1)
    {
        perror("Failed to remove directory");
        return -1;
    }
    return 0; // Directory removed successfully
}

int drop_db(const char *db_name)
{
    if (check_db_exists(db_name))
    {
        char dir[MAX_NAME_LEN + 20];
        snprintf(dir, sizeof(dir), "databases/%s", db_name);
        free_globals(); // Free global variables after dropping the database
        remove_directory(dir);
        return 0;
    }
    else
    {
        printf("Database does not exist\n");
        return -1;
    }
}

int create_tables_file(const char *db_dir)
{
    FILE *file = fopen(db_dir, "wb+");
    if (!file)
    {
        perror("Failed to create .tables file");
        return -1;
    }
    int zero = 0;
    fwrite(&zero, sizeof(int), 1, file); // write table count first
    fflush(file);
    fclose(file);
    return 0;
}

int update_table_count_on_file()
{
    char root[MAX_NAME_LEN + 8];
    snprintf(root, sizeof(root), "%s/.tables", get_root());
    FILE *file = fopen(root, "rb+");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }

    int table_count = get_table_count();
    fwrite(&table_count, sizeof(int), 1, file);
    fflush(file);
    fclose(file);
    return 0;
}

int add_table_to_tables(const char *table_name)
{
    char root[MAX_NAME_LEN + 8];
    snprintf(root, sizeof(root), "%s/.tables", get_root());
    FILE *file = fopen(root, "rb+");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }

    int table_count;
    fread(&table_count, sizeof(int), 1, file);
    if (table_count < 0 || table_count >= MAX_TABLE_COUNT)
    {
        fclose(file);
        printf("Table count exceeds maximum limit\n");
        return -1;
    }
    fseek(file, (MAX_NAME_LEN + 1) * table_count, SEEK_CUR);

    // Write the table name to the file
    char *str_copy = calloc(1, MAX_NAME_LEN + 1);
    if (str_copy == NULL)
    {
        return -1;
    }
    strncpy(str_copy, table_name, strlen(table_name));
    str_copy[MAX_NAME_LEN] = '\0'; // Ensure null termination

    fwrite(str_copy, sizeof(char), MAX_NAME_LEN + 1, file);
    free(str_copy);
    fflush(file);
    fclose(file);
    return 0;
}

int remove_table_from_tables(const char *table_name)
{
    char root[MAX_NAME_LEN + 8];
    snprintf(root, sizeof(root), "%s/.tables", get_root());
    FILE *file = fopen(root, "rb+");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }

    int table_count;
    fread(&table_count, sizeof(int), 1, file);
    if (table_count <= 0)
    {
        fclose(file);
        return 0; // No tables to remove
    }

    char str[MAX_NAME_LEN + 1];
    int found = 0;
    for (int i = 0; i < table_count; i++)
    {
        fread(str, sizeof(char), MAX_NAME_LEN + 1, file);
        if (strcmp(str, table_name) == 0)
        {
            found = 1;
            // shift remaining tables
            for (int j = i; j < table_count - 1; j++)
            {
                fread(str, sizeof(char), MAX_NAME_LEN + 1, file);
                fseek(file, (-MAX_NAME_LEN - 1) * 2, SEEK_CUR);
                fwrite(str, sizeof(char), MAX_NAME_LEN + 1, file);
                fseek(file, MAX_NAME_LEN + 1, SEEK_CUR);
            }
            // Clear the last table name
            char empty_str[MAX_NAME_LEN + 1] = {0};
            fwrite(empty_str, sizeof(char), MAX_NAME_LEN + 1, file);
            break;
        }
    }

    if (found)
    {
        table_count--;
        fseek(file, 0, SEEK_SET);                   // Move to the beginning of the file
        fwrite(&table_count, sizeof(int), 1, file); // Update the table count
    }

    fflush(file);
    fclose(file);
    return found ? 0 : -1; // Return success if a table was removed
}

int does_table_exists(const char *table_name)
{
    char root[MAX_NAME_LEN + 8];
    snprintf(root, sizeof(root), "%s/.tables", get_root());
    FILE *file = fopen(root, "rb");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }
    fseek(file, sizeof(int), SEEK_SET); // Skip the first int which is the table count
    char str[MAX_NAME_LEN + 1];
    while (fread(str, sizeof(char), MAX_NAME_LEN + 1, file) == MAX_NAME_LEN + 1)
    {
        if (strcmp(str, table_name) == 0)
        {
            fclose(file);
            return 1; // Table exists
        }
    }
    fclose(file);
    return 0; // Table does not exist
}

void print_all_columns(const Table *table)
{
    FILE *file = open_file(table->table_name, "bin", "rb");
    if (!file)
    {
        printf("Could not open file for table %s\n", table->table_name);
        return;
    }

    int columns_count = table->columns_count;
    int col_widths[columns_count];

    // Set fixed widths: 12 for INT, 20 for STRING (or column length if longer)
    for (int i = 0; i < columns_count; i++)
    {
        if (table->columns[i].type == INT)
            col_widths[i] = 12;
        else if (table->columns[i].type == STRING)
            col_widths[i] = table->columns[i].lenght > 20 ? table->columns[i].lenght : 20;
        int name_len = strlen(table->columns[i].name);
        if (name_len > col_widths[i])
            col_widths[i] = name_len;
    }

    // Print header
    for (int i = 0; i < columns_count; i++)
        printf("%-*s ", col_widths[i], table->columns[i].name);
    printf("\n");

    // Print separator
    for (int i = 0; i < columns_count; i++)
    {
        for (int j = 0; j < col_widths[i]; j++)
            printf("-");
        printf(" ");
    }
    printf("\n");

    // Print rows
    for (int rec = 0; rec < table->record_size; rec++)
    {
        long row_start = rec * table->row_size_in_bytes;
        long offset = 0;
        for (int col = 0; col < columns_count; col++)
        {
            fseek(file, row_start + offset, SEEK_SET);
            switch (table->columns[col].type)
            {
            case INT:
            {
                int val;
                fread(&val, sizeof(int), 1, file);
                printf("%-*d ", col_widths[col], val);
                offset += sizeof(int);
                break;
            }
            case STRING:
            {
                char *str = (char *)malloc(table->columns[col].lenght + 1);
                if (!str)
                {
                    fclose(file);
                    printf("Memory allocation failed\n");
                    return;
                }
                fread(str, sizeof(char), table->columns[col].lenght, file);
                str[table->columns[col].lenght] = '\0';
                printf("%-*s ", col_widths[col], str);
                free(str);
                offset += table->columns[col].lenght + 1;
                break;
            }
            }
        }
        printf("\n");
    }
    printf("\n");
    fclose(file);
}

void print_values_of(const Table *table, Column **columns, int columns_count)
{
    FILE *file = open_file(table->table_name, "bin", "rb");
    if (!file)
    {
        return;
    }
    int columns_offset[columns_count];
    int col_widths[columns_count];
    for (int i = 0; i < columns_count; i++)
    {
        columns_offset[i] = calculate_offset(table, *columns[i]);
        // left align the column name
        if (columns_offset[i] == -1)
        {
            printf("Column %s not found\n", columns[i]->name);
            fclose(file);
            return;
        }
        if (columns[i]->type == INT)
            col_widths[i] = 12;
        else if (columns[i]->type == STRING)
            col_widths[i] = columns[i]->lenght > 20 ? columns[i]->lenght : 20;
        // Ensure column name fits
        int name_len = strlen(columns[i]->name);
        if (name_len > col_widths[i])
            col_widths[i] = name_len;
    }

    // Print header
    for (int i = 0; i < columns_count; i++)
        printf("%-*s ", col_widths[i], columns[i]->name);
    printf("\n");

    // Print separator
    for (int i = 0; i < columns_count; i++)
    {
        for (int j = 0; j < col_widths[i]; j++)
            printf("-");
        printf(" ");
    }
    printf("\n");

    // Print rows
    for (int i = 0; i < table->record_size; i++)
    {
        for (int j = 0; j < columns_count; j++)
        {
            fseek(file, i * table->row_size_in_bytes + columns_offset[j], SEEK_SET);
            switch (columns[j]->type)
            {
            case INT:
            {
                int val;
                fread(&val, sizeof(int), 1, file);
                printf("%-*d ", col_widths[j], val);
                break;
            }
            case STRING:
            {
                char *str = (char *)malloc(columns[j]->lenght + 1);
                if (!str)
                {
                    fclose(file);
                    return;
                }
                fread(str, sizeof(char), columns[j]->lenght, file);
                str[columns[j]->lenght] = '\0';
                printf("%-*s ", col_widths[j], str);
                free(str);
                break;
            }
            }
        }
        printf("\n");
    }
    printf("\n");
    fclose(file);
}

int create_hashmap_file(const Table *table)
{
    FILE *file = open_file(table->table_name, "hashmap", "wb+");
    if (!file)
    {
        return -1;
    }

    fwrite(&(table->hash->size), sizeof(int), 1, file);
    fwrite(&(table->hash->entries), sizeof(int), 1, file);
    fwrite(&(table->hash->free_hash_spaces_count), sizeof(int), 1, file);
    for (int i = 0; i < DEFAULT_FREE_HASH_SPACES; i++)
    {
        fwrite(&(table->hash->free_hash_spaces[i]), sizeof(long), 1, file);
    }

    fflush(file);
    fclose(file);
    return 0;
}

int create_bin_file(const Table *table)
{
    FILE *file = open_file(table->table_name, "bin", "wb+");
    if (!file)
    {
        return -1;
    }

    fflush(file);
    fclose(file);
    return 0;
}

char *read_all_bin_file(const Table *table)
{
    FILE *file = open_file(table->table_name, "bin", "rb");
    if (!file)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    fread(buffer, sizeof(char), file_size, file);
    fclose(file);
    return buffer;
}

int store_table_metadata(const Table *table)
{
    FILE *file = open_file(table->table_name, "metadata", "wb+");
    if (!file)
    {
        return -1;
    }

    fwrite(table->table_name, sizeof(char), MAX_NAME_LEN, file);
    fwrite(&table->columns_count, sizeof(int), 1, file);
    fwrite(&table->record_size, sizeof(int), 1, file);
    fwrite(&table->row_size_in_bytes, sizeof(int), 1, file);
    fwrite(&table->primary_key, sizeof(Column), 1, file);
    fwrite(&table->free_spaces_count, sizeof(int), 1, file);
    for (int i = 0; i < table->columns_count; i++)
    {
        fwrite(&table->columns[i], sizeof(Column), 1, file);
    }
    for (int i = 0; i < MAX_FREE_SPACES; i++)
    {
        fwrite(&table->free_spaces[i], sizeof(long), 1, file);
    }

    fflush(file);
    fclose(file);
    return 0;
}

int update_table_metadata_record_size(const Table *table)
{
    FILE *file = open_file(table->table_name, "metadata", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, sizeof(char) * MAX_NAME_LEN + sizeof(int), SEEK_SET);
    fwrite(&table->record_size, sizeof(int), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int update_hashmap_file_entries(const Table *table)
{
    FILE *file = open_file(table->table_name, "hashmap", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, sizeof(int), SEEK_SET);
    fwrite(&table->hash->entries, sizeof(int), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int update_table_metadata_free_spaces_count(const Table *table)
{
    FILE *file = open_file(table->table_name, "metadata", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, sizeof(char) * MAX_NAME_LEN + sizeof(int) * 3 + sizeof(Column), SEEK_SET);
    fwrite(&table->free_spaces_count, sizeof(int), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int update_table_metadata_free_spaces(const Table *table)
{
    FILE *file = open_file(table->table_name, "metadata", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, sizeof(char) * MAX_NAME_LEN + sizeof(int) * 4 + sizeof(Column) * (table->columns_count + 1) + sizeof(long) * (table->free_spaces_count - 1), SEEK_SET);
    fwrite(table->free_spaces, sizeof(long), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int update_hashmap_file_free_spaces_count(const Table *table)
{
    FILE *file = open_file(table->table_name, "hashmap", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, sizeof(int) * 2, SEEK_SET);
    fwrite(&table->hash->free_hash_spaces_count, sizeof(int), 1, file);
    fseek(file, sizeof(int) * 2, SEEK_SET);
    int a;
    fread(&a, sizeof(int), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int update_hashmap_file_free_spaces(const Table *table)
{
    FILE *file = open_file(table->table_name, "hashmap", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, sizeof(int) * 3 + sizeof(long) * (table->hash->free_hash_spaces_count - 1), SEEK_SET);
    fwrite(table->hash->free_hash_spaces, sizeof(long), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int insert_to_hashmap_file(const Table *table, HashEntry *he)
{
    FILE *file = open_file(table->table_name, "hashmap", "ab");
    if (!file)
    {
        return -1;
    }

    if (table->hash->free_hash_spaces_count > 0)
    {
        he->hash_entry_pos = table->hash->free_hash_spaces[--table->hash->free_hash_spaces_count];
        table->hash->free_hash_spaces[table->hash->free_hash_spaces_count] = -1; // Mark as used
        table->hash->free_hash_spaces_count--;
    }
    else
    {
        he->hash_entry_pos = ftell(file);
    }
    fseek(file, he->hash_entry_pos, SEEK_SET);
    fwrite(he, sizeof(HashEntry) - sizeof(struct HashEntry *), 1, file);

    fflush(file);
    fclose(file);

    // Update the number of entries in the hashmap file
    if (update_hashmap_file_entries(table) != 0)
    {
        return -1;
    }
    return 0;
}

HashTable *read_hashmap_file(const Table *table)
{
    FILE *file = open_file(table->table_name, "hashmap", "rb");
    if (!file)
    {
        return NULL;
    }

    HashTable *hash = create_hashtable(DEFAULT_TABLE_SIZE);
    fread(&(hash->size), sizeof(int), 1, file);
    fread(&(hash->entries), sizeof(int), 1, file);
    fread(&(hash->free_hash_spaces_count), sizeof(int), 1, file);
    for (int i = 0; i < DEFAULT_FREE_HASH_SPACES; i++)
    {
        fread(&(hash->free_hash_spaces[i]), sizeof(long), 1, file);
    }

    for (int i = 0; i < hash->entries; i++)
    {
        HashEntry *he = (HashEntry *)malloc(sizeof(HashEntry));
        if (!he)
        {
            perror("Failed to allocate memory for hash entry");
            fclose(file);
            return NULL;
        }
        fread(he, sizeof(HashEntry) - sizeof(struct HashEntry *), 1, file);
        he->next = NULL;

        // Insert the hash entry into the hash table
        if (hash->buckets[he->hash % hash->size] == NULL)
        {
            hash->buckets[he->hash % hash->size] = he;
        }
        else
        {
            HashEntry *current = hash->buckets[he->hash % hash->size];
            while (current->next != NULL)
            {
                current = current->next;
            }
            current->next = he;
        }
    }

    fflush(file);
    fclose(file);
    return hash;
}

Table *read_table_metadata(const char *tablename)
{
    FILE *file = open_file(tablename, "metadata", "rb");
    if (!file)
    {
        return NULL;
    }

    Table *table = (Table *)malloc(sizeof(Table));
    if (!table)
    {
        perror("Failed to allocate memory for table");
        fclose(file);
        return NULL;
    }

    fread(table->table_name, sizeof(char), MAX_NAME_LEN, file);
    fread(&table->columns_count, sizeof(int), 1, file);
    fread(&table->record_size, sizeof(int), 1, file);
    fread(&table->row_size_in_bytes, sizeof(int), 1, file);
    fread(&table->primary_key, sizeof(Column), 1, file);
    fread(&table->free_spaces_count, sizeof(int), 1, file);
    for (int i = 0; i < table->columns_count; i++)
    {
        fread(&table->columns[i], sizeof(Column), 1, file);
    }
    for (int i = 0; i < MAX_FREE_SPACES; i++)
    {
        fread(&table->free_spaces[i], sizeof(long), 1, file);
    }

    fflush(file);
    fclose(file);

    // Initialize the hash table
    table->hash = read_hashmap_file(table);
    if (!table->hash)
    {
        free(table);
        return NULL;
    }
    return table;
}

int write_string_to_file(FILE *file, const char *val, int length)
{
    char *str_copy = calloc(1, length + 1);
    if (str_copy == NULL)
    {
        return -1;
    }
    strncpy(str_copy, val, strlen(val));
    str_copy[length] = '\0'; // Null-terminate the string

    fwrite(str_copy, length + 1, 1, file);
    free(str_copy);
    return 0;
}

int delete_entry_from_hashmap_file(const Table *table, const HashEntry *he)
{
    FILE *file = open_file(table->table_name, "hashmap", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, he->hash_entry_pos, SEEK_SET);
    char *deleted_hash_entry = (char *)malloc(sizeof(HashEntry) - sizeof(struct HashEntry *));
    memset(deleted_hash_entry, 0, sizeof(HashEntry) - sizeof(struct HashEntry *)); // Set all bytes to 0
    fwrite(deleted_hash_entry, 1, sizeof(HashEntry) - sizeof(struct HashEntry *), file);

    fflush(file);
    fclose(file);
    free(deleted_hash_entry);
    return 0;
}

int delete_record_from_file(const Table *table, long pos)
{
    FILE *file = open_file(table->table_name, "bin", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, pos, SEEK_SET);
    char *deleted_record = (char *)malloc(table->row_size_in_bytes);
    memset(deleted_record, 0, table->row_size_in_bytes); // Set all bytes to 0
    fwrite(deleted_record, 1, table->row_size_in_bytes, file);

    fflush(file);
    fclose(file);
    free(deleted_record);
    return 0;
}
