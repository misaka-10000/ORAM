#include "Util.h"

using namespace CryptoPP;

size_t Util::key_length = (size_t)AES::DEFAULT_KEYLENGTH;
size_t Util::aes_block_size = (size_t)AES::BLOCKSIZE;
AutoSeededRandomPool Util::prng;
CFB_Mode<AES>::Encryption Util::encrypt_handler;
CFB_Mode<AES>::Decryption Util::decrypt_handler;
std::random_device Util::rd;
std::mt19937 Util::gen(Util::rd());

void Util::aes_encrypt(const std::string& plain, const byte* key, std::string& cipher) {
    byte iv[aes_block_size];
    encrypt_handler.GetNextIV(prng, iv);

    encrypt_handler.SetKeyWithIV(key, key_length, iv, aes_block_size);
    byte cipher_text[plain.length()];
    encrypt_handler.ProcessData(cipher_text, (byte*) plain.c_str(), plain.length());
    cipher = std::string((const char*)iv, aes_block_size) + std::string((const char*)cipher_text, plain.length());
}

void Util::aes_decrypt(const std::string& cipher, const byte* key, std::string& plain) {
    decrypt_handler.SetKeyWithIV(key, key_length, (byte*)cipher.c_str(), aes_block_size);
    size_t cipher_length = cipher.length() - aes_block_size;
    byte plain_text[cipher_length];
    decrypt_handler.ProcessData(plain_text, (byte*)cipher.substr(aes_block_size).c_str(), cipher_length);
    plain = std::string((const char*)plain_text, cipher_length);
}

std::string Util::sha256_hash(const std::string& key, const std::string& salt) {
    byte buf[SHA256::DIGESTSIZE];
    SHA256().CalculateDigest(buf, (byte*) ((key + salt).c_str()), key.length() + salt.length());
    return std::string((const char*)buf, (size_t)SHA256::DIGESTSIZE);
}

std::string Util::generate_random_block(const size_t& length) {
    byte buf[length];
    prng.GenerateBlock(buf, length);
    return std::string((const char*)buf, length);
}