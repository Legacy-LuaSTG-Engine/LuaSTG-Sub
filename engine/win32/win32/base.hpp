#pragma once
#include <string_view>
#include <string>
#include <functional>
#include <stdexcept>
#ifndef NDEBUG
#include <source_location>
#endif

namespace win32 {
	// logger

	void set_logger_writer(std::function<void(std::string_view)> const& writer);

	// HRESULT

	using hresult = int32_t;

#ifndef NDEBUG
#define SL , std::source_location const& sl = std::source_location::current()
#else
#define SL
#endif

	class hresult_error : public std::runtime_error {
	public:
		hresult_error(hresult const code, std::string const& message) : std::runtime_error(message.c_str()), m_code(code) {}
		hresult_error(hresult const code, char const* message) : std::runtime_error(message), m_code(code) {}

		[[nodiscard]] hresult code() const noexcept { return m_code; }
		[[nodiscard]] std::string message() const;
	private:
		hresult m_code{};
	};

	hresult check_hresult(hresult hr SL);

	hresult check_hresult(hresult hr, std::string_view api SL);

	bool check_hresult_as_boolean(hresult hr SL);

	bool check_hresult_as_boolean(hresult hr, std::string_view api SL);

	void check_hresult_throw_if_failed(hresult hr SL);

	void check_hresult_throw_if_failed(hresult hr, std::string_view api SL);
#undef SL

	// com_ptr

	template<typename T>
	class com_ptr {
	public:
		com_ptr() = default;
		com_ptr(com_ptr const& other) : m_object(other.m_object) {
			reference();
		}
		com_ptr(com_ptr&& other) noexcept : m_object(std::exchange(other.m_object, nullptr)) {
		}
		~com_ptr() {
			reset();
		}

		com_ptr& operator=(com_ptr const& other) {
			if (this == &other) {
				return *this;
			}
			release();
			m_object = other.m_object;
			reference();
			return *this;
		}
		com_ptr& operator=(com_ptr&& other) noexcept {
			if (this == &other) {
				return *this;
			}
			release();
			m_object = std::exchange(other.m_object, nullptr);
			return *this;
		}
		com_ptr& operator=(T* const other) noexcept {
			if (m_object == other) {
				return *this;
			}
			release();
			m_object = other;
			reference();
			return *this;
		}

		T* get() const noexcept { return m_object; }
		T* operator->() const noexcept { return m_object; }

		void reset() {
			release();
			m_object = nullptr;
		}
		com_ptr& operator=(std::nullptr_t) {
			reset();
			return *this;
		}

		T** put() {
			reset();
			return &m_object;
		}

		[[nodiscard]] explicit operator bool() const noexcept { return m_object != nullptr; }

		void attach(T* const other) { release(); m_object = other; }
		T* detach() { return std::exchange(m_object, nullptr); }

	private:
		void reference() {
			if (m_object != nullptr) {
				m_object->AddRef();
			}
		}
		void release() {
			if (m_object != nullptr) {
				m_object->Release();
			}
		}

		T* m_object{};
	};

	// CreateInstance

	template<typename ClassId, typename Interface>
	hresult create_instance(ClassId const id, uint32_t const context, Interface** const output_instance) {
		return CoCreateInstance(id, nullptr, context, IID_PPV_ARGS(output_instance));
	}

	template<typename Class, typename Interface>
	hresult create_instance(uint32_t const context, Interface** const output_instance) {
		return CoCreateInstance(__uuidof(Class), nullptr, context, IID_PPV_ARGS(output_instance));
	}
}
