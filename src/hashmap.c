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
        hashmap->buckets[index]->next = he;
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