#include "fcyRefObj.h"
#include <Windows.h>

long _api_InterlockedIncrement(long volatile *add_)
{
    return InterlockedIncrement(add_);
};

long _api_InterlockedDecrement(long volatile *add_)
{
    return InterlockedDecrement(add_);
};
