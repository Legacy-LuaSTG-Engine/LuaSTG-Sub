#pragma once
#include "GameResource/ResourceFont.hpp"
#include "GameResource/Implement/ResourceBaseImpl.hpp"

namespace luastg
{
	class ResourceFontImpl : public ResourceBaseImpl<IResourceFont>
	{
	private:
		Core::ScopeObject<Core::Graphics::IGlyphManager> m_glyphmgr;
		BlendMode m_BlendMode;
		Core::Color4B m_BlendColor;

	public:
		Core::Graphics::IGlyphManager* GetGlyphManager() { return m_glyphmgr.get(); }
		BlendMode GetBlendMode() { return m_BlendMode; }
		void SetBlendMode(BlendMode m) { m_BlendMode = m; }
		Core::Color4B GetBlendColor() { return m_BlendColor; }
		void SetBlendColor(Core::Color4B c) { m_BlendColor = c; }

	public:
		ResourceFontImpl(const char* name, std::string_view hge_path, bool mipmap);
		ResourceFontImpl(const char* name, std::string_view f2d_path, std::string_view tex_path, bool mipmap);
		ResourceFontImpl(const char* name, Core::Graphics::IGlyphManager* p_mgr);
	};
}
