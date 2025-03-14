#pragma once

namespace core {
	template<typename T>
	class SmartReference {
	public:
		SmartReference() noexcept : m_object() {}
		explicit SmartReference(T* const object) noexcept : m_object(object) { reference(); }
		SmartReference(SmartReference const& right) noexcept : m_object(right.m_object) { reference(); }
		SmartReference(SmartReference&& right) noexcept : m_object(right.m_object) { right.m_object = nullptr; }
		~SmartReference() noexcept { release(); }

		SmartReference& operator=(decltype(nullptr)) noexcept { release(); return *this; }
		SmartReference& operator=(T* const object) noexcept { if (m_object != object) { release(); m_object = object; reference(); } return *this; }
		SmartReference& operator=(SmartReference const& right) noexcept { if (this != &right && m_object != right.m_object) { release(); m_object = right.m_object; reference(); } return *this; }
		SmartReference& operator=(SmartReference&& right) noexcept { if (this != &right && m_object != right.m_object) { release(); m_object = right.m_object; right.m_object = nullptr; } return *this; }

		[[nodiscard]] bool operator==(SmartReference const& right) const noexcept { return m_object == right.m_object; }
		[[nodiscard]] bool operator!=(SmartReference const& right) const noexcept { return m_object != right.m_object; }
		[[nodiscard]] explicit operator bool() const noexcept { return m_object != nullptr; }

		T* operator->() const noexcept { return m_object; }
		T* operator*() const noexcept { return m_object; }
		
		SmartReference& attach(T* const object) noexcept { release(); m_object = object; return *this; }
		[[nodiscard]] T* detach() noexcept { T* const object = m_object; m_object = nullptr; return object; }
		SmartReference& reset() noexcept { release(); return *this; }
		[[nodiscard]] T* get() const noexcept { return m_object; }
		[[nodiscard]] T** put() noexcept { release(); return &m_object; }

	private:
		void reference() noexcept { if (m_object) m_object->reference(); }
		void release() noexcept { if (m_object) m_object->release(); m_object = nullptr; }

		T* m_object;
	};
}
