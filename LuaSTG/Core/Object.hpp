#pragma once
#include "Core/Type.hpp"
#include <atomic>

namespace core {
#ifndef NDEBUG
	class ObjectDebugger {
	public:
		static void add(IObject* object);
		static void remove(IObject* object);
		static void check();
	};
#endif // !NDEBUG

	template<typename T = IObject>
	class Object : public T {
	private:
		std::atomic_intptr_t ref_;
	public:
		intptr_t retain() {
			return ref_.fetch_add(1) + 1;
		}
		intptr_t release() {
			intptr_t const ref_last_ = ref_.fetch_sub(1);
			if (ref_last_ <= 1)
			{
				delete this;
			}
			return ref_last_ - 1;
		}
	public:
		Object() : ref_(1) {
		#ifndef NDEBUG
			ObjectDebugger::add(this);
		#endif // !NDEBUG
		}
		virtual ~Object() {
		#ifndef NDEBUG
			ObjectDebugger::remove(this);
		#endif // !NDEBUG
		}
	};
}
