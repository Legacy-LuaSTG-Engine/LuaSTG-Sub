#pragma once
#include <propidl.h>

namespace core::mf_helpers {
    // RAII wrapper for PROPVARIANT
    class PropVariantGuard {
    public:
        PropVariantGuard() { PropVariantInit(&m_var); }
        ~PropVariantGuard() { PropVariantClear(&m_var); }
        
        PropVariantGuard(PropVariantGuard const&) = delete;
        PropVariantGuard& operator=(PropVariantGuard const&) = delete;
        
        PROPVARIANT* get() noexcept { return &m_var; }
        PROPVARIANT const* get() const noexcept { return &m_var; }
        PROPVARIANT* operator->() noexcept { return &m_var; }
        PROPVARIANT const* operator->() const noexcept { return &m_var; }
        
    private:
        PROPVARIANT m_var;
    };
}
