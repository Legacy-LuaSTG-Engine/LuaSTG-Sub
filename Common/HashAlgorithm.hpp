#pragma once
#include <string>
#include <vector>

//#define HashAlgorithm_NoSTL

enum class HashAlgorithmType
{
    MD2,
    MD4,
    MD5,
    SHA1,
    SHA160,
    SHA256,
    SHA384,
    SHA512,
};

class HashAlgorithm
{
private:
    struct Data;
    Data* _data;
public:
    bool is_valid();
    bool start();
    bool hash(uint8_t* data, size_t size);
    bool finish();
public:
    uint8_t* data();
    size_t size();
#ifndef HashAlgorithm_NoSTL
    std::vector<uint8_t> copy_data();
#endif
public:
#ifndef HashAlgorithm_NoSTL
    std::string string(bool space = false, bool upcase = false);
    std::wstring wstring(bool space = false, bool upcase = false);
#endif
public:
    bool hashMemory(const uint8_t* data, size_t size);
#ifndef HashAlgorithm_NoSTL
    bool hashFile(const std::string& path);
    bool hashFile(const std::wstring& path);
#endif
public:
    HashAlgorithm();
    HashAlgorithm(HashAlgorithmType type);
    ~HashAlgorithm();
};
