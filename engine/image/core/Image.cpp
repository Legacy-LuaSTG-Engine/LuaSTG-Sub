#include "core/Image.hpp"
#include "core/implement/ReferenceCounted.hpp"

namespace core {
    class Image final : public implement::ReferenceCounted<IImage> {
    public:
        // IImage

        Vector2U getSize() const noexcept override {
            return m_size;
        }

    private:
        Vector2U m_size;
    };
}
