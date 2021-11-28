#include "slow/ByteArray.hpp"
#include "pch.hpp"

struct float4x4_view
{
    float data[16];
};

int main(int, char**)
{
    slow::object_ptr<slow::IByteArray> blob;
    bool r = slow::createByteArray(~blob, 16 * sizeof(float), 256, false);
    float4x4_view* p = (float4x4_view*)blob->data();
    
    r = blob->writef32(1.0f, 0);
    r = blob->writef32(2.0f, 4);
    r = blob->writef32(3.0f, 8);
    r = blob->writef32(4.0f, 12);
    
    r = blob->writef32(5.0f, 16);
    r = blob->writef32(6.0f, 20);
    r = blob->writef32(7.0f, 24);
    r = blob->writef32(8.0f, 28);
    
    r = blob->writef32(0.1f, 32);
    r = blob->writef32(0.2f, 36);
    r = blob->writef32(0.3f, 40);
    r = blob->writef32(0.4f, 44);
    
    r = blob->writef32(0.5f, 48);
    r = blob->writef32(0.6f, 52);
    r = blob->writef32(0.7f, 56);
    r = blob->writef32(0.8f, 60);
    
    r = blob->writef32(0.9f, 5);
    
    float4x4_view va = {};
    
    r = blob->readf32(va.data + 0, 0);
    r = blob->readf32(va.data + 1, 4);
    r = blob->readf32(va.data + 2, 8);
    r = blob->readf32(va.data + 3, 12);
    
    r = blob->readf32(va.data + 4, 16);
    r = blob->readf32(va.data + 5, 20);
    r = blob->readf32(va.data + 6, 24);
    r = blob->readf32(va.data + 7, 28);
    
    r = blob->readf32(va.data + 8, 32);
    r = blob->readf32(va.data + 9, 36);
    r = blob->readf32(va.data + 10, 40);
    r = blob->readf32(va.data + 11, 44);
    
    r = blob->readf32(va.data + 12, 48);
    r = blob->readf32(va.data + 13, 52);
    r = blob->readf32(va.data + 14, 56);
    r = blob->readf32(va.data + 15, 60);
    
    r = blob->readf32(va.data + 0, 5);
    
    return 0;
}
