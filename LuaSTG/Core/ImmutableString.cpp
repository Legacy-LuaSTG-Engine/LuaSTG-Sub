#include "Core/Type.hpp"
#include "Core/Object.hpp"

namespace core {
	class ImmutableString final
		: public Object<IImmutableString> {
	public:
		// IImmutableString

		[[nodiscard]] bool empty() const noexcept override { return m_string.empty(); }
		[[nodiscard]] char const* data() const noexcept override { return m_string.data(); }
		[[nodiscard]] size_t size() const noexcept override { return m_string.size(); }
		[[nodiscard]] char const* c_str() const noexcept override { return m_string.c_str(); }
		[[nodiscard]] size_t length() const noexcept override { return m_string.length(); }
		[[nodiscard]] StringView view() const noexcept override { return m_string; }

		// ImmutableString

		ImmutableString() = default;
		ImmutableString(ImmutableString const&) = delete;
		ImmutableString(ImmutableString&&) = delete;
		~ImmutableString() = default;

		ImmutableString& operator=(ImmutableString const&) = delete;
		ImmutableString& operator=(ImmutableString&&) = delete;

		void assign(StringView const& view) { m_string.assign(view); }
		void assign(char const* const data, size_t const size) { m_string.assign(data, size); }

	private:
		std::string m_string;
	};
}
namespace core {
	void IImmutableString::create(StringView const& view, IImmutableString** const output) {
		ScopeObject<ImmutableString> s;
		s.attach(new ImmutableString);
		s->assign(view);
		*output = s.detach();
	}
	void IImmutableString::create(char const* const data, size_t const size, IImmutableString** const output) {
		ScopeObject<ImmutableString> s;
		s.attach(new ImmutableString);
		s->assign(data, size);
		*output = s.detach();
	}
}
