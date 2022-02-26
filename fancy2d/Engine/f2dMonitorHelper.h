#pragma once
#include <cstdint>

class f2dMonitorHelper
{
private:
    struct Data;
    Data* _data = nullptr;
public:
    void clear();
    bool refresh();
    uint32_t getCount();
    void* getHandle(uint32_t index);
    int32_t getX(uint32_t index);
    int32_t getY(uint32_t index);
    uint32_t getWidth(uint32_t index);
    uint32_t getHeight(uint32_t index);
    bool isPrimary(uint32_t index);
    uint32_t findFromWindow(void* window, bool primary = false);
    void* getFromWindow(void* window, bool primary = false);
public:
    f2dMonitorHelper();
    ~f2dMonitorHelper();
public:
    static f2dMonitorHelper& get();
};
