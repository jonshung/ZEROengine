#ifndef ZEROENGINE_MURMURHASH3_H
#define ZEROENGINE_MURMURHASH3_H

#include <cstdint>

void MurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out );

void MurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );

#endif