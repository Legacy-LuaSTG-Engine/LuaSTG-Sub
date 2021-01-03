#include "Common/HashAlgorithm.hpp"
#include <fstream>
#include <limits>

#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

struct HashAlgorithm::Data
{
    BCRYPT_ALG_HANDLE hProvider;
    BCRYPT_HASH_HANDLE hHash;
    BYTE* hashObject;
    DWORD hashObjectSize;
    BYTE* hashData;
    DWORD hashDataSize;
    
    HashAlgorithm::Data() :
        hProvider(NULL), hHash(NULL),
        hashObject(NULL), hashObjectSize(0),
        hashData(NULL), hashDataSize(0)
    {
    };
};

#define makeself HashAlgorithm::Data& self = (*((HashAlgorithm::Data*)_data))

bool HashAlgorithm::is_valid()
{
    if (_data == nullptr) return false;
    makeself;
    if (self.hProvider == NULL) return false;
    if (self.hashObject == NULL || self.hashObjectSize == 0) return false;
    if (self.hashData == NULL || self.hashDataSize == 0) return false;
    return true;
};
bool HashAlgorithm::start()
{
    makeself;
    // check
    if (!is_valid())
    {
        // internal state error
        OutputDebugStringA("[E] HashAlgorithm::start internal state error\n");
        return false;
    }
    if (self.hHash != NULL)
    {
        OutputDebugStringA("[E] HashAlgorithm::start missing call to HashAlgorithm::finish\n");
        return false;
    }
    // create hash object
    NTSTATUS status = 0;
    status = BCryptCreateHash(self.hProvider, &self.hHash, (UCHAR*)self.hashObject, self.hashObjectSize, NULL, 0, 0);
    if (status == STATUS_SUCCESS)
    {
        return true;
    }
    else
    {
        self.hHash = NULL;
        OutputDebugStringA("[E] HashAlgorithm::start BCryptCreateHash failed\n");
        return false;
    }
};
bool HashAlgorithm::hash(uint8_t* data, size_t size)
{
    makeself;
    // check
    if (!is_valid() || self.hHash == NULL)
    {
        // internal state error
        OutputDebugStringA("[E] HashAlgorithm::hash internal state error\n");
        return false;
    }
    // hash
    NTSTATUS status = 0;
    status = BCryptHashData(self.hHash, (UCHAR*)data, size, 0);
    if (status == STATUS_SUCCESS)
    {
        return true;
    }
    else
    {
        OutputDebugStringA("[E] HashAlgorithm::hash BCryptHashData failed\n");
        return false;
    }
}
bool HashAlgorithm::finish()
{
    makeself;
    // check
    if (!is_valid() || self.hHash == NULL)
    {
        // internal state error
        OutputDebugStringA("[E] HashAlgorithm::finish internal state error\n");
        return false;
    }
    // hash
    NTSTATUS status = 0;
    bool flag = true;
    // get hash data
    status = BCryptFinishHash(self.hHash, (UCHAR*)self.hashData, self.hashDataSize, 0);
    if (status != STATUS_SUCCESS)
    {
        flag = false;
        OutputDebugStringA("[E] HashAlgorithm::finish BCryptFinishHash failed\n");
    }
    // destroy hash object
    BCryptDestroyHash(self.hHash);
    self.hHash = NULL;
    return flag;
}

uint8_t* HashAlgorithm::data()
{
    if (_data == nullptr) return nullptr;
    makeself;
    return (uint8_t*)self.hashData;
};
size_t HashAlgorithm::size()
{
    if (_data == nullptr) return 0;
    makeself;
    return (size_t)self.hashDataSize;
};
#ifndef HashAlgorithm_NoSTL
std::vector<uint8_t> HashAlgorithm::copy_data()
{
    std::vector<uint8_t> v;
    if (_data == nullptr) return std::move(v);
    makeself;
    v.resize(self.hashDataSize);
    std::memcpy(v.data(), self.hashData, self.hashDataSize);
    return std::move(v);
}
#endif

#ifndef HashAlgorithm_NoSTL
std::string HashAlgorithm::string(bool space, bool upcase)
{
    if (_data == nullptr) return "";
    makeself;
    // make wstirng
    char buffer[3] = { 0 };
    std::string str;
    str.reserve(space ? (2 * self.hashDataSize - 1) : self.hashDataSize);
    for (size_t idx = 0; idx < self.hashDataSize; idx += 1)
    {
        std::snprintf(buffer, 3, upcase ? "%02X" : "%02x", self.hashData[idx]);
        str.append(buffer, 0, 2);
        if (space && (idx + 1) < self.hashDataSize)
        {
            str.push_back(' ');
        }
    }
    return std::move(str);
};
std::wstring HashAlgorithm::wstring(bool space, bool upcase)
{
    if (_data == nullptr) return L"";
    makeself;
    // make wstirng
    wchar_t buffer[3] = { 0 };
    std::wstring str;
    str.reserve(space ? (2 * self.hashDataSize - 1) : self.hashDataSize);
    for (size_t idx = 0; idx < self.hashDataSize; idx += 1)
    {
        std::swprintf(buffer, 3, upcase ? L"%02X" : L"%02x", self.hashData[idx]);
        str.append(buffer, 0, 2);
        if (space && (idx + 1) < self.hashDataSize)
        {
            str.push_back(L' ');
        }
    }
    return std::move(str);
};
#endif

bool HashAlgorithm::hashMemory(const uint8_t* data, size_t size)
{
    makeself;
    // check
    if (!is_valid())
    {
        // internal state error
        OutputDebugStringA("[E] HashAlgorithm::hashMemory internal state error\n");
        return false;
    }
    if (data == nullptr && size > 0)
    {
        // param error
        OutputDebugStringA("[E] HashAlgorithm::hashMemory invalid parameter\n");
        return false;
    }
    // create hash object
    NTSTATUS status = 0;
    BCRYPT_HASH_HANDLE hHashLocal = NULL;
    BYTE* hashObjectLocal = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, self.hashObjectSize);
    if (hashObjectLocal == NULL)
    {
        OutputDebugStringA("[E] HashAlgorithm::hashMemory HeapAlloc failed\n");
        return false;
    }
    status = BCryptCreateHash(self.hProvider, &hHashLocal, (UCHAR*)hashObjectLocal, self.hashObjectSize, NULL, 0, 0);
    if (status == STATUS_SUCCESS)
    {
        bool flag = true;
        // hash
        status = BCryptHashData(hHashLocal, (UCHAR*)data, size, 0);
        if (status != STATUS_SUCCESS)
        {
            flag = false;
            OutputDebugStringA("[E] HashAlgorithm::hashMemory BCryptHashData failed\n");
        }
        // get hash data
        status = BCryptFinishHash(hHashLocal, (UCHAR*)self.hashData, self.hashDataSize, 0);
        if (status != STATUS_SUCCESS)
        {
            flag = false;
            OutputDebugStringA("[E] HashAlgorithm::hashMemory BCryptFinishHash failed\n");
        }
        // destroy hash object
        BCryptDestroyHash(hHashLocal);
        HeapFree(GetProcessHeap(), 0, hashObjectLocal);
        return flag;
    }
    else
    {
        OutputDebugStringA("[E] HashAlgorithm::hashMemory BCryptCreateHash failed\n");
    }
    HeapFree(GetProcessHeap(), 0, hashObjectLocal);
    return false;
};
#ifndef HashAlgorithm_NoSTL
template<typename T>
inline bool _hashFile(HashAlgorithm& last, HashAlgorithm::Data& self, T path)
{
    // check
    if (!last.is_valid())
    {
        // internal state error
        OutputDebugStringA("[E] HashAlgorithm::hashFile internal state error\n");
        return false;
    }
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        // open file error
        OutputDebugStringA("[E] HashAlgorithm::hashFile open file failed\n");
        return false;
    }
    // clean hash data
    NTSTATUS status = 0;
    BCRYPT_HASH_HANDLE hHashLocal = NULL;
    BYTE* hashObjectLocal = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, self.hashObjectSize);
    // create hash object
    status = BCryptCreateHash(self.hProvider, &hHashLocal, (UCHAR*)hashObjectLocal, self.hashObjectSize, NULL, 0, 0);
    if (status == STATUS_SUCCESS)
    {
        bool flag = true;
        // hash
        uint8_t buffer[1024];
        std::streamsize count = file.read((char*)buffer, 1024).gcount();
        constexpr std::streamsize invalid = std::numeric_limits<std::streamsize>::max();
        while (count > 0 && count != invalid)
        {
            status = BCryptHashData(hHashLocal, (UCHAR*)buffer, (ULONG)count, 0); // hash block
            if (status != STATUS_SUCCESS)
            {
                flag = false;
                OutputDebugStringA("[E] HashAlgorithm::hashFile BCryptHashData failed\n");
                break;
            }
            count = file.read((char*)buffer, 1024).gcount(); // read next
        }
        // get hash data
        status = BCryptFinishHash(hHashLocal, (UCHAR*)self.hashData, self.hashDataSize, 0);
        if (status != STATUS_SUCCESS)
        {
            flag = false;
            OutputDebugStringA("[E] HashAlgorithm::hashFile BCryptFinishHash failed\n");
        }
        // destroy hash object
        BCryptDestroyHash(hHashLocal);
        HeapFree(GetProcessHeap(), 0, hashObjectLocal);
        return flag;
    }
    else
    {
        OutputDebugStringA("[E] HashAlgorithm::hashFile BCryptCreateHash failed\n");
    }
    HeapFree(GetProcessHeap(), 0, hashObjectLocal);
    return false;
};
bool HashAlgorithm::hashFile(const std::string& path)
{
    return _hashFile(*this, *_data, path);
}
bool HashAlgorithm::hashFile(const std::wstring& path)
{
    return _hashFile(*this, *_data, path);
}
#endif

HashAlgorithm::HashAlgorithm():
    HashAlgorithm(HashAlgorithmType::SHA256)
{
};
HashAlgorithm::HashAlgorithm(HashAlgorithmType type) : _data(nullptr)
{
    // new self
    _data = new Data;
    if (_data == nullptr)
    {
        OutputDebugStringA("[E] HashAlgorithm::HashAlgorithm error\n");
        return;
    }
    makeself;
    
    NTSTATUS status = 0;
    // open hash provider
    LPCWSTR pid = BCRYPT_SHA256_ALGORITHM;
    switch (type)
    {
    case HashAlgorithmType::MD2:
        pid = BCRYPT_MD2_ALGORITHM;
        break;
    case HashAlgorithmType::MD4:
        pid = BCRYPT_MD4_ALGORITHM;
        break;
    case HashAlgorithmType::MD5:
        pid = BCRYPT_MD5_ALGORITHM;
        break;
    case HashAlgorithmType::SHA1:
    case HashAlgorithmType::SHA160:
        pid = BCRYPT_SHA1_ALGORITHM;
        break;
    case HashAlgorithmType::SHA256:
        pid = BCRYPT_SHA256_ALGORITHM;
        break;
    case HashAlgorithmType::SHA384:
        pid = BCRYPT_SHA384_ALGORITHM;
        break;
    case HashAlgorithmType::SHA512:
        pid = BCRYPT_SHA512_ALGORITHM;
        break;
    default:
        OutputDebugStringA("[E] HashAlgorithm::HashAlgorithm invalid HashAlgorithmType\n");
        break;
    }
    status = BCryptOpenAlgorithmProvider(&self.hProvider, pid, NULL, 0);
    if (status == STATUS_SUCCESS)
    {
        DWORD sizeRead = 0;
        // get hash object size and alloc memory
        status = BCryptGetProperty(self.hProvider, BCRYPT_OBJECT_LENGTH, (UCHAR*)&self.hashObjectSize, sizeof(DWORD), (ULONG*)&sizeRead, 0);
        if (status == STATUS_SUCCESS)
        {
            self.hashObject = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, self.hashObjectSize);
            if (self.hashObject == NULL)
            {
                OutputDebugStringA("[E] HashAlgorithm::HashAlgorithm HeapAlloc BYTE[hashObject] failed\n");
            }
        }
        else
        {
            OutputDebugStringA("[E] HashAlgorithm::HashAlgorithm BCryptGetProperty BCRYPT_OBJECT_LENGTH failed\n");
        }
        // get hash data size and alloc memory
        status = BCryptGetProperty(self.hProvider, BCRYPT_HASH_LENGTH, (UCHAR*)&self.hashDataSize, sizeof(DWORD), (ULONG*)&sizeRead, 0);
        if (status == STATUS_SUCCESS)
        {
            self.hashData = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, self.hashDataSize);
            if (self.hashObject == NULL)
            {
                OutputDebugStringA("[E] HashAlgorithm::HashAlgorithm HeapAlloc BYTE[hashData] failed\n");
            }
        }
        else
        {
            OutputDebugStringA("[E] HashAlgorithm::HashAlgorithm BCryptGetProperty BCRYPT_HASH_LENGTH failed\n");
        }
    }
    else
    {
        self.hProvider = NULL;
        OutputDebugStringA("[E] HashAlgorithm::HashAlgorithm BCryptOpenAlgorithmProvider failed\n");
    }
}
HashAlgorithm::~HashAlgorithm()
{
    if (_data == nullptr) return;
    makeself;
    
    // destroy hash data
    if (self.hashData)
        HeapFree(GetProcessHeap(), 0, self.hashData);
    self.hashData = NULL;
    self.hashDataSize = 0;
    // destroy hash object
    if (self.hHash)
        BCryptDestroyHash(self.hHash);
    self.hHash = NULL;
    if (self.hashObject)
        HeapFree(GetProcessHeap(), 0, self.hashObject);
    self.hashObject = NULL;
    self.hashObjectSize = 0;
    // close provider
    if (self.hProvider)
        BCryptCloseAlgorithmProvider(self.hProvider, 0);
    self.hProvider = NULL;
    
    // delete self
    delete _data;
    _data = nullptr;
}
