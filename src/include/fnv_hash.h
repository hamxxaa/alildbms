#ifndef FNV_HASH_H
#define FNV_HASH_H

#include <stdint.h>
#include <stddef.h>

// FNV-1a hash for 32-bit systems
uint32_t fnv1a_hash_int(int key);
uint32_t fnv1a_hash_str(const char *key);
uint32_t fnv1a_hash_bytes(const void *data, size_t length);

#endif // FNV_HASH_H