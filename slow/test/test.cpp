#pragma comment(lib, "Bcrypt.lib")

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <bcrypt.h>

namespace BCrypt {
    enum class Operation : uint32_t {
        Invalid = 0x00000000,
        Cipher = 0x00000001,
        Hash = 0x00000002,
        AsymmetricEncryption = 0x00000004,
        SecretAgreement = 0x00000008,
        Signature = 0x00000010,
        RandomNumberGenerator = 0x00000020,
    };
    
    std::vector<std::wstring> EnumAlgorithms(Operation op) {
        std::vector<std::wstring> ret;
        ULONG cnt = 0;
        BCRYPT_ALGORITHM_IDENTIFIER* list = nullptr;
        BCryptEnumAlgorithms((ULONG)op, &cnt, &list, 0);
        for (ULONG idx = 0; idx < cnt; idx++) {
            ret.push_back(list[idx].pszName);
        }
        BCryptFreeBuffer(list);
        return ret;
    }
    
    std::vector<std::wstring> EnumProviders(const std::wstring& al) {
        std::vector<std::wstring> ret;
        ULONG cnt = 0;
        BCRYPT_PROVIDER_NAME* list = nullptr;
        BCryptEnumProviders(al.c_str(), &cnt, &list, 0);
        for (ULONG idx = 0; idx < cnt; idx++) {
            ret.push_back(list[idx].pszProviderName);
        }
        BCryptFreeBuffer(list);
        return ret;
    }
};

int main() {
    auto al = BCrypt::EnumAlgorithms(BCrypt::Operation::Cipher);
    for (auto& a : al) {
        std::wcout << a << std::endl;
        auto pr = BCrypt::EnumProviders(a);
        for (auto& p : pr) {
            std::wcout << L"    " << p << std::endl;
        }
    }
    
    NTSTATUS status = 0;
    BOOL bretv = TRUE;
    {
        BCRYPT_ALG_HANDLE alg_handle = NULL;
        status = BCryptOpenAlgorithmProvider(&alg_handle, BCRYPT_AES_ALGORITHM, NULL, 0);
        {
            ULONG keyobjlen = 0;
            ULONG keyobjlenw = 0;
            status = BCryptGetProperty(alg_handle, BCRYPT_OBJECT_LENGTH, (PUCHAR)&keyobjlen, sizeof(keyobjlen), &keyobjlenw, 0);
            ULONG blockobjlen = 0;
            ULONG blockobjlenw = 0;
            status = BCryptGetProperty(alg_handle, BCRYPT_BLOCK_LENGTH, (PUCHAR)&blockobjlen, sizeof(blockobjlen), &blockobjlenw, 0);
            status = BCryptSetProperty(alg_handle, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
            PUCHAR keyobj = (PUCHAR)HeapAlloc(GetProcessHeap(), 0, keyobjlen);
            PUCHAR blockobj = (PUCHAR)HeapAlloc(GetProcessHeap(), 0, blockobjlen);
            {
                UCHAR key[] = {
                    'N', 'M', 'S', 'L', 'N', 'M', 'S', 'L',
                    'N', 'M', 'S', 'L', 'N', 'M', 'S', 'L',
                };
                BCRYPT_KEY_HANDLE key_handle = NULL;
                status = BCryptGenerateSymmetricKey(alg_handle, &key_handle, keyobj, keyobjlen, key, sizeof(key), 0);
                {
                    
                }
                status = BCryptDestroyKey(key_handle);
                key_handle = NULL;
            }
            bretv = HeapFree(GetProcessHeap(), 0, keyobj);
            bretv = HeapFree(GetProcessHeap(), 0, blockobj);
        }
        status = BCryptCloseAlgorithmProvider(alg_handle, 0);
        alg_handle = NULL;
    }
    
    return 0;
}
