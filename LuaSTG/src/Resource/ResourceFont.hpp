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
		Core::ScopeObject<Core::Graphics::IGlyphManager> m_glyphmgr;
		BlendMode m_BlendMode;
		Core::Color4B m_BlendColor;

	public:
		Core::Graphics::IGlyphManager* GetGlyphManager() { return m_glyphmgr.get(); }
		BlendMode GetBlendMode() const noexcept { return m_BlendMode; }
		void SetBlendMode(BlendMode m) noexcept { m_BlendMode = m; }
		Core::Color4B GetBlendColor() const noexcept { return m_BlendColor; }
		void SetBlendColor(Core::Color4B c) noexcept { m_BlendColor = c; }

	public:
		ResFont(const char* name, std::string_view hge_path, bool mipmap);
		ResFont(const char* name, std::string_view f2d_path, std::string_view tex_path, bool mipmap);
		ResFont(const char* name, Core::Graphics::IGlyphManager* p_mgr);
		~ResFont();
	};
}
