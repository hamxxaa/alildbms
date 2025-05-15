#include "table.h"
#include "file_io.h"
#include <stdio.h>
#include <string.h>

int calculate_row_size_in_bytes(const Column *columns, const int columns_count)
{
    int size = 0;
    for (int i = 0; i < columns_count; i++)
    {
        switch (columns[i].type)
        {
        case INT:
            size += sizeof(int);
            break;

        case STRING:
            size += columns[i].lenght + 1; // +1 for null terminator
            break;
        }
    }
    return size;
}

int create_initial_files_for_table(Table *table)
{
    // Create binary file for data to be stored
    if (create_bin_file(table) != 0)
    {
        perror("Failed to create binary file");
        return -1;
    }

    // Create metadata file
    if (store_table_metadata(table) != 0)
    {
        perror("Failed to store table metadata");
        return -1;
    }

    // Initialize the hash table file
    if (create_hashmap_file(table) != 0)
    {
        perror("Failed to create hashmap file");
        return -1;
    }

    return 0;
}

Table *create_table(const char *table_name, const Column *columns, const int columns_count, const Column primary_key)
{
    Table *table = (Table *)malloc(sizeof(Table));
    if (!table)
    {
        perror("Failed to allocate table");
        return NULL;
    }

    strncpy(table->table_name, table_name, MAX_NAME_LEN);
    for (int i = 0; i < columns_count; i++)
    {
        strncpy(table->columns[i].name, columns[i].name, MAX_NAME_LEN);
        table->columns[i].type = columns[i].type;
        table->columns[i].lenght = columns[i].lenght;
    }
    table->record_size = 0;
    table->columns_count = columns_count;
    table->primary_key = primary_key;
    table->free_spaces_count = 0;
    for (int i = 0; i < MAX_FREE_SPACES; i++)
    {
        table->free_spaces[i] = -1;
    }
    table->hash = create_hashtable(DEFAULT_TABLE_SIZE);
    if (!table->hash)
    {
        perror("Failed to create hash table");
        free(table);
        return NULL;
    }
    table->row_size_in_bytes = calculate_row_size_in_bytes(columns, columns_count);
    create_initial_files_for_table(table);

    return table;
}

int insert_record(Table *table, ...)
{
    va_list args;
    va_start(args, table);

    FILE *file = open_file(table->table_name, "bin", "ab");
    if (!file)
    {
        return -1;
    }

    Key key;
    uint32_t hash;
    long pos;
    if (table->free_spaces_count > 0)
    {
        // Use the first free space
        pos = table->free_spaces[table->free_spaces_count - 1];
        table->free_spaces[table->free_spaces_count - 1] = -1; // Mark as used
        table->free_spaces_count--;
        if (update_table_metadata_free_spaces(table) != 0)
        {
            printf("Failed to update free spaces in metadata file\n");
            fclose(file);
            va_end(args);
            return -1;
        }
        fseek(file, pos, SEEK_SET);
    }
    else
    {
        // Append to the end of the file
        pos = ftell(file);
    }

    // Write each column value based on its type
    for (int i = 0; i < table->columns_count; i++)
    {
        switch (table->columns[i].type)
        {
        case INT:
        {
            int val = va_arg(args, int);
            fwrite(&val, sizeof(int), 1, file);
            if (cmpcolumns(table->columns[i], table->primary_key) == 0)
            {
                key.int_key = val;
                hash = fnv1a_hash_int(val);
            }
            break;
        }
        case STRING:
        {
            char *str = va_arg(args, char *);
            if (write_string_to_file(file, str, table->columns[i].lenght) != 0)
            {
                printf("Failed to write string to file\n");
                fclose(file);
                va_end(args);
                return -1;
            }
            if (table->columns[i].name == table->primary_key.name)
            {
                key.char_key = str;
                hash = fnv1a_hash_str(str);
            }
            break;
        }
        }
    }

    HashEntry *he = create_hash_entry(table->hash, key, hash, pos);
    table->record_size++;

    // Update the record size in the metadata file
    if (update_table_metadata_record_size(table) != 0)
    {
        perror("Failed to update record size in metadata file");
        free(he);
        fclose(file);
        va_end(args);
        return -1;
    }

    // Update the hash table file
    if (insert_to_hashmap_file(table, he) != 0)
    {
        perror("Failed to insert to hashmap file");
        free(he);
        fclose(file);
        va_end(args);
        return -1;
    }

    va_end(args);
    fflush(file);
    fclose(file);
    return 0;
}

char *search_record_by_key(const Table *table, ...)
{
    va_list args;
    va_start(args, table);

    FILE *file = open_file(table->table_name, "bin", "rb");
    if (!file)
    {
        return NULL;
    }

    long pos = find_record_position(table, args);
    if (pos == -1)
    {
        printf("Table not found\n");
        fclose(file);
        va_end(args);
        return NULL;
    }

    if (pos == -2)
    {
        printf("Invalid data type\n");
        fclose(file);
        va_end(args);
        return NULL;
    }

    if (pos == -3)
    {
        printf("Hash entry could not be created\n");
        fclose(file);
        va_end(args);
        return NULL;
    }

    // Read data
    fseek(file, pos, SEEK_SET);
    char *buffer = (char *)malloc(table->row_size_in_bytes);
    if (buffer == NULL)
    {
        perror("Memory allocation failed");
        free(buffer);
        fclose(file);
        return NULL;
    }

    int bytes_read = fread(buffer, 1, table->row_size_in_bytes, file);
    if (bytes_read != table->row_size_in_bytes)
    {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fflush(file);
    fclose(file);
    va_end(args);
    return buffer;
}

void print_row_readable(const Table *table, const char *data)
{
    int intdata;
    char *chardata;

    for (int i = 0; i < table->columns_count; i++)
    {
        printf("%s: ", table->columns[i].name);
        switch (table->columns[i].type)
        {
        case INT:
            intdata = *(int *)data;
            printf("%d\n", intdata);
            data += sizeof(int);
            break;

        case STRING:
            chardata = malloc((table->columns[i].lenght + 1) * sizeof(char));
            memcpy(chardata, data, table->columns[i].lenght + 1);
            printf("%s\n", chardata);
            free(chardata);
            data += table->columns[i].lenght + 1; // Move pointer to the next column
            break;
        }
    }
    printf("\n");
}

int check_column_exists(const Table *table, const Column column)
{
    for (int i = 0; i < table->columns_count; i++)
    {
        if (strcmp(table->columns[i].name, column.name) == 0)
        {
            return 1; // Column exists
        }
    }
    return 0; // Column does not exist
}

int check_column_exists_by_name(const Table *table, const char *column_name)
{
    for (int i = 0; i < table->columns_count; i++)
    {
        if (strcmp(table->columns[i].name, column_name) == 0)
        {
            return 1; // Column exists
        }
    }
    return 0; // Column does not exist
}

int cmpcolumns(const Column a, const Column b)
{
    return strcmp(a.name, b.name);
}

long find_record_position(const Table *table, va_list args)
{
    if (!table)
    {
        return -1;
    }

    Key key;
    uint32_t hash;

    if (table->primary_key.type == INT)
    {
        key.int_key = va_arg(args, int);
        hash = fnv1a_hash_int(key.int_key);
    }
    else if (table->primary_key.type == STRING)
    {
        key.char_key = va_arg(args, char *);
        hash = fnv1a_hash_str(key.char_key);
    }
    else
    {
        return -2;
    }

    HashEntry *he = find_right_entry_in_bucket(table->hash, key, hash);
    if (he == NULL)
    {
        return -3;
    }

    return he->file_pos;
}

int calculate_offset(const Table *table, const Column column)
{
    int offset = 0;
    for (int i = 0; i < table->columns_count; i++)
    {
        if (cmpcolumns(table->columns[i], column) == 0)
        {
            return offset;
        }
        else
        {
            switch (table->columns[i].type)
            {
            case INT:
                offset += sizeof(int);
                break;

            case STRING:
                offset += table->columns[i].lenght + 1; // +1 for null terminator
                break;
            }
        }
    }
    return -1; // Column not found
}

int update_record(const Table *table, const Column column, ...)
{

    if (!check_column_exists(table, column))
    {
        printf("Column does not exist\n");
        return -1;
    }

    va_list args;
    va_start(args, column);

    FILE *file = open_file(table->table_name, "bin", "rb+");
    if (!file)
    {
        return -1;
    }

    long pos = find_record_position(table, args);
    if (pos < 0)
    {
        printf("Record not found\n");
        fclose(file);
        va_end(args);
        return -1;
    }

    int offset = calculate_offset(table, column);
    if (offset < 0)
    {
        printf("Column not found\n");
        fclose(file);
        va_end(args);
        return -1;
    }
    fseek(file, pos + offset, SEEK_SET);

    switch (column.type)
    {
    case INT:
    {
        int val = va_arg(args, int);
        fwrite(&val, sizeof(int), 1, file);
        break;
    }
    case STRING:
    {
        char *str = va_arg(args, char *);
        if (write_string_to_file(file, str, column.lenght) != 0)
        {
            printf("Failed to write string to file\n");
            fclose(file);
            va_end(args);
            return -1;
        }
        break;
    }
    }

    fflush(file);
    fclose(file);
    va_end(args);
    return 0;
}

int delete_record(Table *table, ...)
{
    //TODO: delete from hashmap
    va_list args;
    va_start(args, table);

    FILE *file = open_file(table->table_name, "bin", "rb+");
    if (!file)
    {
        return -1;
    }

    long pos = find_record_position(table, args);
    if (pos < 0)
    {
        printf("Record not found\n");
        fclose(file);
        va_end(args);
        return -1;
    }

    fseek(file, pos, SEEK_SET);
    char *deleted_record = (char *)malloc(table->row_size_in_bytes);
    memset(deleted_record, 0, table->row_size_in_bytes); // Set all bytes to 0
    fwrite(deleted_record, 1, table->row_size_in_bytes, file);

    table->record_size--;
    table->free_spaces[table->free_spaces_count] = pos;
    table->free_spaces_count++;
    if (update_table_metadata_free_spaces(table) != 0)
    {
        printf("Failed to update free spaces in metadata file\n");
        free(deleted_record);
        fclose(file);
        va_end(args);
        return -1;
    }

    free(deleted_record);
    fflush(file);
    fclose(file);
    va_end(args);
    return 0;
}
