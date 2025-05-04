#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>

struct Table; // Forward declaration of Table struct

typedef union
{
    int int_key;
    char *char_key;
} Key;

typedef struct HashEntry
{
    Key key;
    uint32_t hash;
    long file_pos;
    struct HashEntry *next;
} HashEntry;

typedef struct
{
    int size;
    int entries;
    HashEntry **buckets;
} HashTable;

/**
 * @brief Create a hash table with the given size.
 *
 * @param size The size of the hash table.
 * @return HashTable* Pointer to the created hash table. NULL if failed.
 */
HashTable *create_hashtable(const int size);

/**
 * @brief Create a hash entry with the given key, hash value, and file position and store it in the corresponding index of hashtable.
 *
 * @param hashmap The hashmap to which the hash entry belongs.
 * @param key The key of the hash entry.
 * @param hash The hash value of the key.
 * @param file_pos The file position of the record.
 * @return HashEntry* Pointer to the created hash entry. NULL if failed.
 */
HashEntry *create_hash_entry(HashTable *hashmap, const Key key, const uint32_t hash, const long file_pos);

/**
 * @brief Find the right entry in the bucket for the given key and hash value.
 *
 * @param hash The hash table.
 * @param key The key to search for.
 * @param hash_value The hash value of the key.
 * @return HashEntry* Pointer to the found hash entry. NULL if not found.
 */
HashEntry *find_right_entry_in_bucket(HashTable *hash, const Key key, const uint32_t hash_value);

#endif