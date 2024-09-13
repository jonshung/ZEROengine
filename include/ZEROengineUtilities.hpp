#ifndef ZEROENGINE_UTILITIES_H
#define ZEROENGINE_UTILitIES_H

#include <cstdint>
#include "ZEROengineMurmurHash3.hpp"

/**
 * @brief Simple general purpose hashing using MurmurHash3.
 * 
 * @tparam T The hashing object's type
 * @param seed Previous hashes. If not used, place 0.
 * @param v The hashing object.
 * @return std::size_t 
 */
template <class T>
inline std::size_t hash_combine(const std::size_t& seed, const T& v) {
    std::size_t ret = 0;
    MurmurHash3_x86_32(reinterpret_cast<const char*>(&v), sizeof(T), seed, &ret);
    return ret;
}

/**
 * @brief Simple general purpose hashing using MurmurHash3, with explicit sizing.
 * 
 * @tparam T The hashing object's type
 * @param seed Previous hashes. If not used, place 0.
 * @param v The hashing object.
 * @param size The object size.
 * @return std::size_t 
 */
template <class T>
inline std::size_t hash_combine(const std::size_t& seed, const T& v, const uint64_t &size) {
    std::size_t ret = 0;
    MurmurHash3_x86_32(reinterpret_cast<const char*>(&v), size, seed, &ret);
    return ret;
}

#endif