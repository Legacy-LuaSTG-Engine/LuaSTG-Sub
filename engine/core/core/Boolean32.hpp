#pragma once

namespace core {
	class Boolean32 {
	public:
		Boolean32() noexcept : m_value() {}
		explicit Boolean32(bool const value) noexcept : m_value(value ? 1 : 0) {}
		Boolean32(Boolean32 const&) noexcept = default;
		Boolean32(Boolean32&&) noexcept = default;
		~Boolean32() noexcept = default;

		Boolean32& operator=(Boolean32 const&) noexcept = default;
		Boolean32& operator=(Boolean32&&) noexcept = default;
		Boolean32& operator=(bool const value) noexcept { m_value = value ? 1 : 0; return *this; }

		[[nodiscard]] bool value() const noexcept { return m_value != 0; }
		Boolean32& value(bool const value) noexcept { m_value = value ? 1 : 0; return *this; }

		[[nodiscard]] bool operator==(Boolean32 const right) const noexcept { return m_value == right.m_value; }
		[[nodiscard]] bool operator!=(Boolean32 const right) const noexcept { return m_value != right.m_value; }

		[[nodiscard]] explicit operator bool() const noexcept { return m_value != 0; }

		[[nodiscard]] static Boolean32 of(bool const value) noexcept { return Boolean32(value); }
	private:
		int m_value;
	};

	static_assert(sizeof(int) == 4);
	static_assert(sizeof(int) == sizeof(Boolean32));
}
