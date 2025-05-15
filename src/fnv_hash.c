#include "fnv_hash.h"

// FNV constants for 32-bit hashes
#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U

// Hash a 32-bit integer
uint32_t fnv1a_hash_int(int key)
{
    uint32_t hash = FNV_OFFSET_BASIS;
    const unsigned char *bytes = (const unsigned char *)&key;

    for (size_t i = 0; i < sizeof(int); i++)
    {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

// Hash a null-terminated string
uint32_t fnv1a_hash_str(const char *key)
{
    uint32_t hash = FNV_OFFSET_BASIS;

    while (*key)
    {
        hash ^= (unsigned char)*key++;
        hash *= FNV_PRIME;
    }

    return hash;
}

// Hash arbitrary bytes (e.g., structs)
uint32_t fnv1a_hash_bytes(const void *data, size_t length)
{
    uint32_t hash = FNV_OFFSET_BASIS;
    const unsigned char *bytes = (const unsigned char *)data;

    for (size_t i = 0; i < length; i++)
    {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }

    return hash;
}