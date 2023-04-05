#ifndef SEAL_ORAM_UTIL_H
#define SEAL_ORAM_UTIL_H

#include <string>
#include <random>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>

using namespace CryptoPP;

class Util {
public:
    static void aes_encrypt(const std::string& plain, const byte* key, std::string& cipher);
    static void aes_decrypt(const std::string& cipher, const byte* key, std::string& plain);
    static std::string sha256_hash(const std::string& key, const std::string& salt);
    static std::string generate_random_block(const size_t& length);

    template<typename T>
    static void psuedo_random_permute(T* items, size_t n) {
        for (size_t i = n - 1; i > 0; --i) {
            size_t j = std::uniform_int_distribution<size_t>(0, i)(gen);
            T tmp = items[i];
            items[i] = items[j];
            items[j] = tmp;
        }
    }

    static size_t rand_int(size_t n) {
        return std::uniform_int_distribution<size_t>(0, n - 1)(gen);
        //return rand() % n;
    }

    static size_t key_length;
    static size_t aes_block_size;
    static AutoSeededRandomPool prng;
    static CFB_Mode<AES>::Encryption encrypt_handler;
    static CFB_Mode<AES>::Decryption decrypt_handler;

    static std::random_device rd;
    static std::mt19937 gen;
};

#endif //SEAL_ORAM_UTIL_H
