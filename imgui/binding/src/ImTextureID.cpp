#include "lua_imgui_binding.hpp"
#include <cassert>
#include <format>

using std::string_view_literals::operator ""sv;

namespace {
	constexpr auto prefix = "imgui.ImTextureID:"sv;
}

namespace imgui::binding {
	std::string ImTextureIDUtils::marshal(ImTextureID const texture_id) {
		return std::format("imgui.ImTextureID:{}"sv, texture_id);
	}
	ImTextureID ImTextureIDUtils::unmarshal(std::string_view const value) {
		ImTextureID texture_id{ ImTextureID_Invalid };
		if (!value.starts_with(prefix)) {
			return texture_id;
		}
		auto const result = std::from_chars(value.data() + prefix.size(), value.data() + value.size(), texture_id);
		if (result.ec != std::errc{}) {
			assert(false);
			texture_id = ImTextureID_Invalid;
		}
		return texture_id;
	}
}
