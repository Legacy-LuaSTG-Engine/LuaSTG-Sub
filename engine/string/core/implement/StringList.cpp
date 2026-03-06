#include "core/implement/StringList.hpp"

namespace core::implement {
    uint32_t StringList::getCount() const noexcept {
        return static_cast<uint32_t>(m_data.size());
    }

    StringView StringList::getView(const uint32_t index) const noexcept {
        if (index < m_data.size()) {
            return m_data[index];
        }
        return {};
    }

    bool StringList::getString(const uint32_t index, IImmutableString** const output) const noexcept {
        if (index >= m_data.size()) {
            return false;
        }
        IImmutableString::create(m_data[index], output);
        return true;
    }

    std::vector<std::string>& StringList::getData() { return m_data; }
}
