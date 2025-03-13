#pragma once
#include "core/WeakReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "core/implement/WeakReferenceSource.hpp"

namespace core::implement {
	template<typename Target>
	class WeakReference final : public ReferenceCounted<IWeakReference> {
	public:
		// IObject

		bool queryInterface(InterfaceId const interface_id, void** output) override {
			if (interface_id == getInterfaceId<IReferenceCounted>()) {
				reference();
				*output = static_cast<IReferenceCounted*>(this);
				return true;
			}
			if (interface_id == getInterfaceId<IObject>()) {
				reference();
				*output = static_cast<IObject*>(this);
				return true;
			}
			if (interface_id == getInterfaceId<IWeakReference>()) {
				reference();
				*output = static_cast<IWeakReference*>(this);
				return true;
			}
			return false;
		}

		// IWeakReference

		bool resolve(InterfaceId const interface_id, void** const output) override {
			if (auto const counter = m_object_counter; counter->strong.load() > 0) {
				return m_object->queryInterface(interface_id, output);
			}
			return false;
		}

		// WeakReference

		explicit WeakReference(IObject* object) : m_object_counter(static_cast<Target*>(object)->m_counter), m_object(object) {
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
		IObject* m_object;
	};
}
