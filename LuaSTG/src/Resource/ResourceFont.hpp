#pragma once
#include "ResourceBase.hpp"
#include "fcyMath.h"
#include "Core/Graphics/Font.hpp"

namespace LuaSTGPlus
{
	// 纹理字体
	class ResFont : public Resource
	{
	public:
		enum class FontAlignHorizontal  // 水平对齐
		{
			Left,
			Center,
			Right
		};
		enum class FontAlignVertical  // 垂直对齐
		{
			Top,
			Middle,
			Bottom
		};
		
	private:
		LuaSTG::Core::ScopeObject<LuaSTG::Core::Graphics::IGlyphManager> m_glyphmgr;
		BlendMode m_BlendMode;
		fcyColor m_BlendColor;

	public:
		LuaSTG::Core::Graphics::IGlyphManager* GetGlyphManager() { return m_glyphmgr.get(); }
		BlendMode GetBlendMode() const noexcept { return m_BlendMode; }
		void SetBlendMode(BlendMode m) noexcept { m_BlendMode = m; }
		fcyColor GetBlendColor() const noexcept { return m_BlendColor; }
		void SetBlendColor(fcyColor c) noexcept { m_BlendColor = c; }

	public:
		ResFont(const char* name, std::string_view hge_path, bool mipmap);
		ResFont(const char* name, std::string_view f2d_path, std::string_view tex_path, bool mipmap);
		ResFont(const char* name, LuaSTG::Core::Graphics::IGlyphManager* p_mgr);
		~ResFont();
	};
}
