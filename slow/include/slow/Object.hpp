#pragma once
#include "slow/Type.hpp"

// reference object
namespace slow
{
    struct IObject
    {
        virtual i32 reserve() = 0;
        virtual i32 release() = 0;
        virtual ~IObject() {}
    };
    
    template<typename T>
    class Object : public T
    {
    private:
        volatile i32 _count;
    public:
        i32 reserve()
        {
            _count += 1;
            return _count;
        }
        i32 release()
        {
            _count -= 1;
            if (_count < 1)
            {
                const i32 temp_ = _count;
                delete this;
                return temp_;
            }
            return _count;
        }
    public:
        Object() : _count(1) {}
        virtual ~Object() {}
    };
    
    template<typename T = IObject>
    class object_ptr
    {
    private:
        T* _p;
        inline void _reserve() { if (_p) { _p->reserve(); } }
        inline void _release() { if (_p) { _p->release(); } _p = nullptr; }
    public:
        T* operator->() const { return _p; }
        T* get() const { return _p; }
        T** getAddress() { return &_p; }
        object_ptr& operator=(T* p) { _release(); _p = p; _reserve(); return *this; }
        object_ptr& setDirect(T* p) { _release(); _p = p; return *this; }
        void reset() { _release(); }
        T* getOwnership() { T* p = _p; _p = nullptr; return p; }
        T** getNewAddress() { _release(); return &_p; }
        T** operator~() { _release(); return &_p; }
        operator bool() const { return _p != nullptr; }
        operator T* () const { return _p; }
    public:
        object_ptr() : _p(nullptr) {}
        object_ptr(T* p) : _p(p) { _reserve(); }
        object_ptr(object_ptr& r) : _p(r._p) { _reserve(); }
        object_ptr(const object_ptr& r) : _p(r._p) { _reserve(); }
        object_ptr(object_ptr&& r) : _p(r._p) { r._p = nullptr; }
        object_ptr(const object_ptr&&) = delete;
        ~object_ptr() { _release(); }
    };
}
