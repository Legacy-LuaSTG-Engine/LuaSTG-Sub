#include "GameResource/ResourceFont.hpp"
#include "Core/Object.hpp"
#include "Core/FileManager.hpp"
#include "utility/utf.hpp"
#include "utility/encoding.hpp"
#include "AppFrame.h"
#include "fcyMisc/fcyStringHelper.h"
#include "fcyParser/fcyXml.h"

namespace LuaSTGPlus
{
	class hgeFont
		: public Core::Object<Core::Graphics::IGlyphManager>
	{
	private:
		Core::ScopeObject<Core::Graphics::ITexture2D> m_texture;
		std::unordered_map<uint32_t, Core::Graphics::GlyphInfo> m_map;
		float m_line_height;
		
	private:
		void readDefine(const std::wstring& data, std::wstring& tex)
		{
			std::vector<std::wstring> tLines;
			fcyStringHelper::StringSplit(data, L"\n", true, tLines);
			for (auto& i : tLines)
			{
				i = fcyStringHelper::Trim(i);
			}

			// 第一行必须是HGEFONT
			if (tLines.size() <= 1 || tLines[0] != L"[HGEFONT]")
				throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");

			for (size_t i = 1; i < tLines.size(); ++i)
			{
				std::wstring& tLine = tLines[i];
				if (tLine.size() == 0)
					continue;

				std::wstring::size_type tPos;
				if (std::string::npos == (tPos = tLine.find_first_of(L"=")))
					throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
				std::wstring tKey = tLine.substr(0, tPos);
				std::wstring tValue = tLine.substr(tPos + 1, tLine.size() - tPos - 1);
				if (tKey == L"Bitmap")
					tex = tValue;
				else if (tKey == L"Char")
				{
					wchar_t c;
					int c_hex;
					float x, y, w, h, left_offset, right_offset;
					if (7 != swscanf_s(tValue.c_str(), L"\"%c\",%f,%f,%f,%f,%f,%f", &c, 1, &x, &y, &w, &h, &left_offset, &right_offset))
					{
						if (7 != swscanf_s(tValue.c_str(), L"%X,%f,%f,%f,%f,%f,%f", &c_hex, &x, &y, &w, &h, &left_offset, &right_offset))
							throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
						c = static_cast<wchar_t>(c_hex);
					}

					// 计算到f2d字体偏移量
					Core::Graphics::GlyphInfo const tInfo = {
						.texture_index = 0,
						.texture_rect = Core::RectF(x, y, x + w, y + h),
						.size = Core::Vector2F(w, h),
						.position = Core::Vector2F(left_offset, h),
						.advance = Core::Vector2F(w + left_offset + right_offset, 0),
					};
					if (m_map.find(c) != m_map.end())
						throw fcyException("ResFont::HGEFont::readDefine", "Duplicated character defination.");
					m_map.emplace(c, tInfo);
				}
				else
					throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
			}

			if (tex.empty())
				throw fcyException("ResFont::HGEFont::readDefine", "Bad file format.");
		}

	public:
		float getLineHeight() { return m_line_height + 1.0f; } // ?
		float getAscender() { return m_line_height; }
		float getDescender() { return 0.0f; }

		uint32_t getTextureCount() { return 1; }
		Core::Graphics::ITexture2D* getTexture(uint32_t index)
		{
			if (index == 0)
			{
				return m_texture.get();
			}
			assert(false); return nullptr;
		}

		bool cacheGlyph(uint32_t) { return true; }
		bool cacheString(Core::StringView) { return true; }
		bool flush() { return true; }

		bool getGlyph(uint32_t codepoint, Core::Graphics::GlyphInfo* p_ref_info, bool)
		{
			auto it = m_map.find(codepoint);
			if (it != m_map.end())
			{
				*p_ref_info = it->second;
				return true;
			}
			return false;
		}

	public:
		hgeFont(std::string_view path, bool mipmap)
			: m_line_height(0.0f)
		{
			// 打开 HGE 字体定义文件
			std::vector<uint8_t> src;
			if (!GFileManager().loadEx(path, src))
			{
				spdlog::error("[luastg] 加载 HGE 纹理字体失败，无法加载字体定义文件 '{}'", path);
				throw std::runtime_error("hgeFont::hgeFont");
			}
			

			std::wstring tex_wpath;
			readDefine(
				utility::encoding::to_wide(
					std::string_view((char*)src.data(), src.size())
				),
				tex_wpath);

			/*
			std::string_view font_define((char*)src.data(), src.size());
			if (font_define.empty())
			{
				spdlog::error("[luastg] 加载 HGE 纹理字体失败，字体定义文件 '{}' 格式无效", path);
				throw std::runtime_error("hgeFont::hgeFont");
			}

			// 切成行
			std::vector<std::string_view> lines;
			{
				size_t offset = 0;
				auto pos = font_define.find_first_of("\n", offset);
				while (pos != std::string_view::npos)
				{
					lines.push_back(std::string_view(
						font_define.data() + offset,
						pos - offset));
					offset = pos + 1;
				}
				lines.push_back(font_define.substr(offset));
			}
			if (lines.size() < 1)
			{
				spdlog::error("[luastg] 加载 HGE 纹理字体失败，字体定义文件 '{}' 格式无效", path);
				throw std::runtime_error("hgeFont::hgeFont");
			}

			// 检查第一行
			if (lines[0] != "[HGEFONT]")
			{
				spdlog::error("[luastg] 加载 HGE 纹理字体失败，字体定义文件 '{}' 格式无效", path);
				throw std::runtime_error("hgeFont::hgeFont");
			}

			// 解析每一行
			std::string_view texture;
			for (size_t i = 1; i < lines.size(); i += 1)
			{
				std::string_view line = lines[i];
				if (line.empty())
				{
					continue; // 跳过空白行
				}
				if (line.starts_with("Bitmap="))
				{
					texture = line.substr(7);
				}
				else if (line.starts_with("Char="))
				{
					std::string_view data = line.substr(5);
					char buffer[8]{}; // UTF-8 真的会有这么长的吗
					int c_hex = 0;
					float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
					float left_offset = 0.0f, right_offset = 0.0f;
					if (7 != std::sscanf(data.data(), "\"%7s\",%f,%f,%f,%f,%f,%f", buffer, &x, &y, &w, &h, &left_offset, &right_offset))
					{
						if (7 != std::sscanf(data.data(), "%X,%f,%f,%f,%f,%f,%f", &c_hex, &x, &y, &w, &h, &left_offset, &right_offset))
						{
							spdlog::warn("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 中发现无法解析的行：{}", path, line);
							continue; // 润
						}
					}
					uint32_t codepoint = 0;
					if (c_hex)
					{
						codepoint = (uint32_t)c_hex;
					}
					else
					{
						char32_t c = 0;
						utf::utf8reader reader(buffer);
						if (!reader(c))
						{
							spdlog::warn("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 中发现无法识别的字符：{}", path, line);
							continue; // 润
						}
						codepoint = (uint32_t)c;
					}
					if (m_map.find(codepoint) != m_map.end())
					{
						spdlog::warn("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 中发现重复的字符：{}", path, line);
						continue; // 润
					}
					// 计算字体数据
					LuaSTG::Core::Graphics::GlyphInfo const glyph_info = {
						.texture_index = 0,
						.texture_rect = LuaSTG::Core::RectF(x, y, x + w, y + h),
						.size = LuaSTG::Core::Vector2F(w, h),
						.position = LuaSTG::Core::Vector2F(left_offset, h),
						.advance = LuaSTG::Core::Vector2F(w + left_offset + right_offset, 0),
					};
					m_map.emplace(codepoint, glyph_info);
				}
				else
				{
					spdlog::warn("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 中发现无法识别的行格式：{}", path, line);
				}
			}
			//*/

			// 加载纹理
			std::string texture(utility::encoding::to_utf8(tex_wpath));
			if (GFileManager().containEx(texture))
			{
				if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(texture, mipmap, ~m_texture))
				{
					spdlog::error("[luastg] 加载 HGE 纹理字体失败，无法加载纹理 '{}'", texture);
					throw std::runtime_error("hgeFont::hgeFont");
				}
			}
			else
			{
				// 切换到同级文件夹
				std::filesystem::path wide_path(utility::encoding::to_wide(path));
				wide_path.remove_filename();
				wide_path /= utility::encoding::to_wide(texture);
				std::string texture_path(std::move(utility::encoding::to_utf8(wide_path.wstring())));
				if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(texture_path, mipmap, ~m_texture))
				{
					spdlog::error("[luastg] 加载 HGE 纹理字体失败，无法加载纹理 '{}'", texture_path);
					throw std::runtime_error("hgeFont::hgeFont");
				}
			}

			// 进一步处理
			for (auto& v : m_map)
			{
				// 计算最高行作为LineHeight
				m_line_height = std::max(m_line_height, v.second.size.y);
				// 修正纹理坐标
				v.second.texture_rect.a.x /= (float)m_texture->getSize().x;
				v.second.texture_rect.b.x /= (float)m_texture->getSize().x;
				v.second.texture_rect.a.y /= (float)m_texture->getSize().y;
				v.second.texture_rect.b.y /= (float)m_texture->getSize().y;
			}
		}
		~hgeFont()
		{
		}
	};

	class f2dFont
		: public Core::Object<Core::Graphics::IGlyphManager>
	{
	private:
		Core::ScopeObject<Core::Graphics::ITexture2D> m_texture;
		std::unordered_map<uint32_t, Core::Graphics::GlyphInfo> m_map;
		float m_line_height;
		float m_ascender;
		float m_descender;

	private:
		Core::Vector2F readVec2Str(const std::wstring& Str)
		{
			Core::Vector2F tRet;
			if (2 != swscanf_s(Str.c_str(), L"%f,%f", &tRet.x, &tRet.y))
				throw fcyException("f2dFontTexProvider::readVec2Str", "String format error.");
			return tRet;
		}
		void loadDefine(fcyXmlDocument& Xml)
		{
			float const tXScale = 1.0f / (float)m_texture->getSize().x;
			float const tYScale = 1.0f / (float)m_texture->getSize().y;

			fcyXmlElement* pRoot = Xml.GetRootElement();

			if (pRoot->GetName() != L"f2dTexturedFont")
				throw fcyException("f2dFontTexProvider::loadDefine", "Invalid file, root node name not match.");

			fcyXmlElement* pMeasureNode = pRoot->GetFirstNode(L"Measure");
			if (!pMeasureNode)
				throw fcyException("f2dFontTexProvider::loadDefine", "Invalid file, node 'Measure' not found.");

			fcyXmlElement* pCharList = pRoot->GetFirstNode(L"CharList");

			// 读取度量值
			m_line_height = (float)_wtof(pMeasureNode->GetAttribute(L"LineHeight").c_str());
			m_ascender = (float)_wtof(pMeasureNode->GetAttribute(L"Ascender").c_str());
			m_descender = (float)_wtof(pMeasureNode->GetAttribute(L"Descender").c_str());

			// 读取字符表
			fcyXmlElementList tNodeList = pCharList->GetNodeByName(L"Item");
			uint32_t tSubNodeCount = tNodeList.GetCount();
			for (uint32_t i = 0; i < tSubNodeCount; ++i)
			{
				fcyXmlElement* pSub = tNodeList[i];

				const std::wstring& tChar = pSub->GetAttribute(L"Char");
				if (tChar.length() != 1)
					throw fcyException("f2dFontTexProvider::loadDefine", "Invalid file, invalid character in CharList.");

				Core::Vector2F const Advance = readVec2Str(pSub->GetAttribute(L"Advance"));
				Core::Vector2F const BrushPos = readVec2Str(pSub->GetAttribute(L"BrushPos"));
				Core::Vector2F const GlyphSize = readVec2Str(pSub->GetAttribute(L"Size"));
				Core::Vector2F GlyphPosA = readVec2Str(pSub->GetAttribute(L"Pos"));
				Core::Vector2F GlyphPosB = GlyphPosA + readVec2Str(pSub->GetAttribute(L"Size"));
				GlyphPosA.x *= tXScale;
				GlyphPosA.y *= tYScale;
				GlyphPosB.x *= tXScale;
				GlyphPosB.y *= tYScale;

				Core::Graphics::GlyphInfo const glyph_info = {
						.texture_index = 0,
						.texture_rect = Core::RectF(GlyphPosA.x, GlyphPosA.y, GlyphPosB.x, GlyphPosB.y),
						.size = GlyphSize,
						.position = BrushPos,
						.advance = Advance,
				};
				m_map[(uint32_t)tChar[0]] = glyph_info;
			}
		}

	public:
		float getLineHeight() { return m_line_height; }
		float getAscender() { return m_ascender; }
		float getDescender() { return m_descender; }

		uint32_t getTextureCount() { return 1; }
		Core::Graphics::ITexture2D* getTexture(uint32_t index)
		{
			if (index == 0)
			{
				return m_texture.get();
			}
			assert(false); return nullptr;
		}

		bool cacheGlyph(uint32_t) { return true; }
		bool cacheString(Core::StringView) { return true; }
		bool flush() { return true; }

		bool getGlyph(uint32_t codepoint, Core::Graphics::GlyphInfo* p_ref_info, bool)
		{
			auto it = m_map.find(codepoint);
			if (it != m_map.end())
			{
				*p_ref_info = it->second;
				return true;
			}
			return false;
		}

	public:
		f2dFont(std::string_view path, std::string_view raw_texture_path, bool mipmap)
			: m_line_height(0.0f)
			, m_ascender(0.0f)
			, m_descender(0.0f)
		{
			// 打开 fancy2d 字体定义文件
			std::vector<uint8_t> src;
			if (!GFileManager().loadEx(path, src))
			{
				spdlog::error("[luastg] 加载 fancy2d 纹理字体失败，无法加载字体定义文件 '{}'", path);
				throw std::runtime_error("f2dFont::f2dFont");
			}

			// 解析
			fcyXmlDocument tXml(utility::encoding::to_wide(
				std::string_view((char*)src.data(), src.size())
			));
			loadDefine(tXml);

			// 加载纹理
			if (GFileManager().containEx(raw_texture_path))
			{
				if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(raw_texture_path, mipmap, ~m_texture))
				{
					spdlog::error("[luastg] 加载 fancy2d 纹理字体失败，无法加载纹理 '{}'", raw_texture_path);
					throw std::runtime_error("f2dFont::f2dFont");
				}
			}
			else
			{
				// 切换到同级文件夹
				std::filesystem::path wide_path(utility::encoding::to_wide(path));
				wide_path.remove_filename();
				wide_path /= utility::encoding::to_wide(raw_texture_path);
				std::string texture_path(std::move(utility::encoding::to_utf8(wide_path.wstring())));
				if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(texture_path, mipmap, ~m_texture))
				{
					spdlog::error("[luastg] 加载 fancy2d 纹理字体失败，无法加载纹理 '{}'", texture_path);
					throw std::runtime_error("f2dFont::f2dFont");
				}
			}
		}
		~f2dFont()
		{
		}
	};

	ResFont::ResFont(const char* name, std::string_view hge_path, bool mipmap)
		: Resource(ResourceType::SpriteFont, name)
		, m_BlendMode(BlendMode::MulAlpha)
		, m_BlendColor(Core::Color4B(0xFFFFFFFFu))
	{
		m_glyphmgr.attach(new hgeFont(hge_path, mipmap));
	}
	ResFont::ResFont(const char* name, std::string_view f2d_path, std::string_view tex_path, bool mipmap)
		: Resource(ResourceType::SpriteFont, name)
		, m_BlendMode(BlendMode::MulAlpha)
		, m_BlendColor(Core::Color4B(0xFFFFFFFFu))
	{
		m_glyphmgr.attach(new f2dFont(f2d_path, tex_path, mipmap));
	}
	ResFont::ResFont(const char* name, Core::Graphics::IGlyphManager* p_mgr)
		: Resource(ResourceType::SpriteFont, name)
		, m_glyphmgr(p_mgr)
		, m_BlendMode(BlendMode::MulAlpha)
		, m_BlendColor(Core::Color4B(0xFFFFFFFFu))
	{
	}
	ResFont::~ResFont()
	{
	}
}
