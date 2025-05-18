#include "hashmap.h"
#include "table.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

HashTable *create_hashtable(const int size)
{
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht)
    {
        perror("Failed to allocate memory for hash table");
        return NULL;
    }
    ht->size = size;
    ht->entries = 0;
    ht->buckets = (HashEntry **)calloc(size, sizeof(HashEntry *));
    if (!ht->buckets)
    {
        perror("Failed to allocate memory for hash table buckets");
        free(ht);
        return NULL;
    }
    ht->free_hash_spaces_count = 0;
    for (int i = 0; i < DEFAULT_FREE_HASH_SPACES; i++)
    {
        ht->free_hash_spaces[i] = -1;
    }
    return ht;
}

HashEntry *create_hash_entry(HashTable *hashmap, const Key key, const uint32_t hash, const long file_pos)
{
    HashEntry *he = (HashEntry *)malloc(sizeof(HashEntry));
    if (!he)
    {
        perror("Failed to allocate memory for hash entry");
        return NULL;
    }
    he->key = key;
    he->hash = hash;
    he->file_pos = file_pos;
    he->next = NULL;

    int index = hash % hashmap->size;
    if (hashmap->buckets[index])
    {
        HashEntry *current = hashmap->buckets[index];
        while (current->next != NULL)
        {
            current = current->next;
        }
        current = he;
    }
    else
    {
        hashmap->buckets[index] = he;
    }
    hashmap->entries++;

    return he;
}

HashEntry *find_right_entry_in_bucket(HashTable *hash, const Key key, const uint32_t hash_value)
{
    int index = hash_value % hash->size;
    HashEntry *he = hash->buckets[index];

    while (he)
    {
        if (he->key.int_key == key.int_key || he->key.char_key == key.char_key)
        {
            return he;
        }
        he = he->next;
    }
    return NULL; // Not found
}

int delete_hash_entry(HashTable *hash, HashEntry *entry)
{
    if (!hash || !entry)
    {
        return -1; // Invalid parameters
    }

    uint32_t hash_value = entry->hash;
    int index = hash_value % hash->size;
    HashEntry *current = hash->buckets[index];
    HashEntry *prev = NULL;
    while (current)
    {
        if (current == entry)
        {
            if (prev)
            {
                prev->next = current->next;
            }
            else
            {
                hash->buckets[index] = current->next;
            }
            hash->entries--;
            hash->free_hash_spaces[hash->free_hash_spaces_count] = entry->hash_entry_pos;
            hash->free_hash_spaces_count++;
            free(current);
            return 0; // Successfully deleted
        }
        prev = current;
        current = current->next;
    }
    return -1; // Entry not found
}

int free_hashtable(HashTable *hash)
{
    if (!hash)
    {
        return -1; // Invalid parameter
    }
    for (int i = 0; i < hash->size; i++)
    {
        HashEntry *current = hash->buckets[i];
        while (current)
        {
            HashEntry *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(hash->buckets);
    free(hash);
    return 0;
}