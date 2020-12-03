#pragma once
#include <cstdint>

namespace slow {
	// basic reference count interface
	struct Ref {
		// retain ownership
		virtual int32_t retain() = 0;
		// release ownership
		virtual int32_t release() = 0;
		
		virtual ~Ref() {}
	};
	
	// baka⑨ pointer
	template<class T>
	class Pointer {
	private:
		T* _p;
	private:
		inline void _add() { if (_p != nullptr) { _p->retain(); } }
		inline void _sub() { if (_p != nullptr) { _p->release(); } }
		inline void _free() { if (_p != nullptr) { _p->release(); _p = nullptr; } }
	public:
		// compare
		bool operator== (const Pointer& p) const { return (_p == p._p); }
		// assignment from raw pointer, retain new
		Pointer& operator= (T* p) { set(p); return *this; }
		// assignment from smart pointer, retain new
		Pointer& operator= (const Pointer& p) { set(p._p); return *this; }
		// call member method
		T* operator-> () const { return _p; }
		// get raw pointer
		T* operator* () const { return _p; }
		// get pointer of raw pointer
		T** operator& () { return &_p; }
	public:
		// release old, retain new
		void set(T* p) {
			if (_p != p) {
				_free();
				_p = p;
				_add();
			}
		}
		// release old
		void rawset(T* p) {
			if (_p != p) {
				_free();
				_p = p;
			}
		}
		// release old
		void reset() { _free(); }
		// release and return pointer
		T* remove() { T* r = _p; _free(); return r; }
		// remove old, !not release
		void clear() { _p = nullptr; }
		// swap two Pointer
		void swap(Pointer& p) { T* _tmp = _p; _p = p._p; p._p = _tmp; }
	public:
		Pointer() : _p(nullptr) {}
		Pointer(T* p) : _p(p) { _add(); }
		Pointer(const Pointer& p) : _p(p._p) { _add(); }
		~Pointer() { _free(); }
	};
};
