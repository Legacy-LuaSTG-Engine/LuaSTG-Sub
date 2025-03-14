#pragma once
#include "core/WeakReference.hpp"
#include "core/implement/ReferenceCounted.hpp"

namespace core::implement {
	class WeakReference final : public ReferenceCounted<IWeakReference> {
	public:
		// IWeakReference

		Boolean32 resolve(UUID const& uuid, void** const output) override {
			assert(output != nullptr);
			auto const counter = m_object_counter;
			if (auto const last_strong = counter->strong.fetch_add(1); last_strong > 0) {
				auto const result = m_object->queryInterface(uuid, output);
				counter->strong.fetch_sub(1);
				return result;
			}
			counter->strong.fetch_sub(1);
			*output = nullptr;
			return Boolean32::of(false);
		}

		// WeakReference

		explicit WeakReference(ReferenceCounter* const counter, IReferenceCounted* const object) : m_object_counter(counter), m_object(object) {
			assert(m_object_counter != nullptr);
			assert(m_object != nullptr);
			m_object_counter->weak.fetch_add(1);
		}
		WeakReference(WeakReference const&) = delete;
		WeakReference(WeakReference&&) = delete;
		~WeakReference() override {
			auto const counter = m_object_counter;
			if (auto const last_weak = counter->weak.fetch_sub(1); last_weak == 1) {
				delete counter;
			}
		}

		WeakReference& operator=(WeakReference const&) = delete;
		WeakReference& operator=(WeakReference&&) = delete;

	private:
		ReferenceCounter* m_object_counter;
		IReferenceCounted* m_object;
	};
}
