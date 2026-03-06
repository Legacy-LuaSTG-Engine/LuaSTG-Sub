#pragma once
#include "core/ImmutableStringList.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <vector>
#include <string>

namespace core::implement {
    class StringList final : public ReferenceCounted<IImmutableStringList> {
    public:
        [[nodiscard]] uint32_t getCount() const noexcept override;
        [[nodiscard]] StringView getView(uint32_t index) const noexcept override;
        [[nodiscard]] bool getString(uint32_t index, IImmutableString** output) const noexcept override;

        std::vector<std::string>& getData();

    private:
        std::vector<std::string> m_data;
    };
}
