#include "file_io.h"

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

int create_hashmap_file(Table *table)
{
    FILE *file = open_file(table->table_name, "hashmap", "wb+");
    if (!file)
    {
        return -1;
    }

    fwrite(&(table->hash->size), sizeof(int), 1, file);
    fwrite(&(table->hash->entries), sizeof(int), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int create_bin_file(Table *table)
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

int store_table_metadata(Table *table)
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
    for (int i = 0; i < table->columns_count; i++)
    {
        fwrite(&table->columns[i], sizeof(Column), 1, file);
    }

    fflush(file);
    fclose(file);
    return 0;
}

int update_table_metadata_record_size(Table *table)
{
    FILE *file = open_file(table->table_name, "metadata", "rb+");
    if (!file)
    {
        return -1;
    }

    fseek(file, sizeof(int) + sizeof(char) * MAX_NAME_LEN, SEEK_SET);
    fwrite(&table->record_size, sizeof(int), 1, file);

    fflush(file);
    fclose(file);
    return 0;
}

int update_hashmap_file_entries(Table *table)
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

int insert_to_hashmap_file(Table *table, const HashEntry *he)
{
    FILE *file = open_file(table->table_name, "hashmap", "ab");
    if (!file)
    {
        return -1;
    }

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

HashTable *read_hashmap_file(Table *table)
{
    FILE *file = open_file(table->table_name, "hashmap", "rb");
    if (!file)
    {
        return NULL;
    }

    HashTable *hash = create_hashtable(DEFAULT_TABLE_SIZE);
    fread(&(hash->size), sizeof(int), 1, file);
    fread(&(hash->entries), sizeof(int), 1, file);

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
    for (int i = 0; i < table->columns_count; i++)
    {
        fread(&table->columns[i], sizeof(Column), 1, file);
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
