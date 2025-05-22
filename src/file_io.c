#include "file_io.h"
#include <string.h>

FILE *open_file(const char *table_name, const char *exit, const char *mode)
{
    char filename[MAX_NAME_LEN + 23];
    snprintf(filename, sizeof(filename), "%ss/%s.%s", exit, table_name, exit);
    FILE *file = fopen(filename, mode);
    if (!file)
    {
        perror("Failed to create data file");
        return NULL;
    }

    return file;
}

int add_table_to_tables(const char *table_name)
{
    FILE *file = fopen(".tables", "wb+");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }
    // Write the table name to the file
    char *str_copy = calloc(1, MAX_NAME_LEN + 1);
    if (str_copy == NULL)
    {
        return -1;
    }
    strncpy(str_copy, table_name, strlen(table_name));

    fwrite(str_copy, sizeof(char), MAX_NAME_LEN, file);
    free(str_copy);
    fflush(file);
    fclose(file);
    return 0;
}

int does_table_exists(const char *table_name)
{
    FILE *file = fopen(".tables", "wb+");
    if (!file)
    {
        perror("Failed to open .tables file");
        return -1;
    }
    char str[MAX_NAME_LEN + 1];
    while (fread(str, sizeof(char), MAX_NAME_LEN, file) == MAX_NAME_LEN)
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
