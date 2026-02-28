#include "VideoBindingHelpers.hpp"
#include "AppFrame.h"

namespace luastg::binding::video {
    core::IVideoDecoder* getDecoderFromResourceName(const char* name) noexcept {
        auto texture = LRES.FindTexture(name);
        if (!texture) {
            return nullptr;
        }
        
        auto texture2d = texture->GetTexture();
        if (!texture2d) {
            return nullptr;
        }
        
        return getDecoderFromTexture(texture2d);
    }
}
