#pragma once

#include <cstdint>
#include <atomic>

namespace slow {
    class IObject {
    public:
        virtual int32_t retain() = 0;
        
        virtual int32_t release() = 0;
        
        virtual int32_t refcount() = 0;
    
    public:
        virtual ~IObject() = default;
    };
    
    template<typename T>
    class Object : public T {
    private:
        std::atomic_int32_t _ref;
    public:
        int32_t retain() {
            return _ref.fetch_add(1) + 1;
        };
        
        int32_t release() {
            const int32_t cnt_ = _ref.fetch_sub(1);
            if (cnt_ <= 1) {
                delete this;
            }
            return cnt_ - 1;
        };
        
        int32_t refcount() {
            return _ref.load();
        };
    public:
        Object() : _ref(1) {};
        
        virtual ~Object() = default;
    };
    
    template<class T = IObject>
    class Pointer {
    private:
        T* _p = nullptr;
    private:
        inline void _add() {
            if (_p != nullptr) {
                _p->retain();
            }
        };
        
        inline void _sub() {
            if (_p != nullptr) {
                _p->release();
            }
        };
        
        inline void _free() {
            if (_p != nullptr) {
                _p->release();
                _p = nullptr;
            }
        };
    public:
        bool operator==(const T* p) const { return _p == p; }
        
        bool operator==(const Pointer& p) const { return _p == p._p; }
        
        Pointer& operator=(const T* p) {
            set(p);
            return *this;
        }
        
        Pointer& operator=(const Pointer& p) {
            set(p._p);
            return *this;
        }
        
        T* operator->() const { return _p; }
        
        T* operator*() const { return _p; }
        
        // T** operator&() { return &_p; }
        
        T** operator~() {
            _free();
            return &_p;
        }
        
        explicit operator T*() const { return _p; }
        
        explicit operator bool() const { return _p != nullptr; }
    
    public:
        void set(T* p) {
            if (_p != p) {
                _free();
                _p = p;
                _add();
            }
        }
        
        void rawset(T* p) {
            if (_p != p) {
                _free();
                _p = p;
            }
        }
        
        T* get() { return _p; }
        
        T** getref() { return &_p; }
        
        void reset() { _free(); }
        
        T* remove() {
            T* r_ = _p;
            _free();
            return r_;
        }
        
        void swap(Pointer& p) {
            T* v_ = _p;
            _p = p._p;
            p._p = v_;
        }
        
        void clear() { _p = nullptr; }
    
    public:
        Pointer() = default;
        
        explicit Pointer(T* p) : _p(p) { _add(); }
        
        Pointer(const Pointer& p) : _p(p._p) { _add(); }
        
        Pointer(Pointer&& p) noexcept { swap(p); }
        
        ~Pointer() { _free(); }
    };
}
