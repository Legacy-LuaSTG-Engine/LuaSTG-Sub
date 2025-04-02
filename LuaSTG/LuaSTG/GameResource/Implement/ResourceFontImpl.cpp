#include "GameResource/Implement/ResourceFontImpl.hpp"
#include "Core/FileManager.hpp"
#include "utility/utf.hpp"
#include "pugixml.hpp"
#include "AppFrame.h"
#include "utf8.hpp"

namespace luastg
{
	class hgeFont
		: public Core::Object<Core::Graphics::IGlyphManager>
	{
	private:
		Core::ScopeObject<Core::Graphics::ITexture2D> m_texture;
		std::unordered_map<uint32_t, Core::Graphics::GlyphInfo> m_map;
		float m_line_height;

	private:
		bool readDefine(std::string_view path, std::string_view font_define, std::string_view& texture)
		{
			auto errorInvalidFormat = [&path]()
			{
				spdlog::error("[luastg] 加载 HGE 纹理字体失败，字体定义文件 '{}' 格式无效", path);
			};

			// 第一次检查

			if (font_define.empty())
			{
				errorInvalidFormat();
				return false;
			}

			// 分割为行方便处理

			std::vector<std::string_view> lines;
			{
				std::string_view view = font_define;
				size_t pos = view.find_first_of("\n");
				while (pos != std::string_view::npos)
				{
					std::string_view line = view.substr(0, pos);
					if (!line.empty() && line.back() == '\r')
					{
						line = line.substr(0, line.size() - 1);
					}
					if (!line.empty())
					{
						lines.push_back(line);
					}
					view = view.substr(pos + 1);
					pos = view.find_first_of("\n");
				}
				if (!view.empty() && view.back() == '\r')
				{
					view = view.substr(0, view.size() - 1);
				}
				if (!view.empty())
				{
					lines.push_back(view);
				}
			}

			// 第二次检查

			if (lines.empty())
			{
				errorInvalidFormat();
				return false;
			}
			if (lines[0] != "[HGEFONT]")
			{
				errorInvalidFormat();
				return false;
			}

			// 逐行解析

			auto errorLineParser = [&path](std::string_view line)
			{
				spdlog::error("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 中发现无法解析的行：'{}'", path, line);
			};

			struct CharData
			{
				int state;
				char32_t code;
				float x;
				float y;
				float w;
				float h;
				float left_offset;
				float right_offset;
			};

			auto readCharData = [&path, &errorLineParser](std::string_view line) -> CharData
			{
				// Char="A",0,0,1,1,2,2

				std::string_view data = line.substr(5);
				CharData cd{};

				size_t qp1 = data.find_first_of('\"');
				size_t qp2 = data.find_last_of('\"');

				size_t sp1 = std::string_view::npos;
				size_t sp2 = std::string_view::npos;
				size_t sp3 = std::string_view::npos;
				size_t sp4 = std::string_view::npos;
				size_t sp5 = std::string_view::npos;
				size_t sp6 = std::string_view::npos;

				if (qp1 != std::string_view::npos && qp2 != std::string_view::npos && (qp2 - qp1) > 1)
				{
					/*                              */ sp1 = data.find_first_of(',', qp2 + 1);
					if (sp1 != std::string_view::npos) sp2 = data.find_first_of(',', sp1 + 1);
					if (sp2 != std::string_view::npos) sp3 = data.find_first_of(',', sp2 + 1);
					if (sp3 != std::string_view::npos) sp4 = data.find_first_of(',', sp3 + 1);
					if (sp4 != std::string_view::npos) sp5 = data.find_first_of(',', sp4 + 1);
					if (sp5 != std::string_view::npos) sp6 = data.find_first_of(',', sp5 + 1);

					if (sp6 == std::string_view::npos)
					{
						errorLineParser(line);
						return {};
					}

					std::string_view str(data.substr(qp1 + 1, qp2 - (qp1 + 1)));
					utf::utf8reader reader(str.data(), str.size());
					if (!reader.step(cd.code))
					{
						spdlog::error("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 行 '{}' 中发现无法识别的字符：'{}'", path, line, str);
						return {};
					}
				}
				else if (qp1 == std::string_view::npos && qp2 == std::string_view::npos)
				{
					/*                              */ sp1 = data.find_first_of(',');
					if (sp1 != std::string_view::npos) sp2 = data.find_first_of(',', sp1 + 1);
					if (sp2 != std::string_view::npos) sp3 = data.find_first_of(',', sp2 + 1);
					if (sp3 != std::string_view::npos) sp4 = data.find_first_of(',', sp3 + 1);
					if (sp4 != std::string_view::npos) sp5 = data.find_first_of(',', sp4 + 1);
					if (sp5 != std::string_view::npos) sp6 = data.find_first_of(',', sp5 + 1);

					if (sp6 == std::string_view::npos)
					{
						errorLineParser(line);
						return {};
					}

					std::string hex(data.substr(0, sp1));
					try
					{
						cd.code = (char32_t)std::stoi(hex, nullptr, 16);
					}
					catch (std::exception const& e)
					{
						spdlog::error("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 行 '{}' 中发现无法识别的码点：'{}' ({})", path, line, hex, e.what());
						return {};
					}
				}
				else
				{
					errorLineParser(line);
					return {};
				}

				std::string ce1;
				std::string ce2;
				std::string ce3;
				std::string ce4;
				std::string ce5;
				std::string ce6;
				std::string ce7;

				if (sp1 != std::string_view::npos) ce1 = data.substr(0, sp1);
				if (sp2 != std::string_view::npos) ce2 = data.substr(sp1 + 1, sp2 - (sp1 + 1));
				if (sp3 != std::string_view::npos) ce3 = data.substr(sp2 + 1, sp3 - (sp2 + 1));
				if (sp4 != std::string_view::npos) ce4 = data.substr(sp3 + 1, sp4 - (sp3 + 1));
				if (sp5 != std::string_view::npos) ce5 = data.substr(sp4 + 1, sp5 - (sp4 + 1));
				if (sp6 != std::string_view::npos) ce6 = data.substr(sp5 + 1, sp6 - (sp5 + 1));
				/*                              */ ce7 = data.substr(sp6 + 1);

				if (ce1.empty() || ce2.empty() || ce3.empty() || ce4.empty() || ce5.empty() || ce6.empty() || ce7.empty())
				{
					errorLineParser(line);
					return {};
				}

				try
				{
					cd.x = std::stof(ce2);
					cd.y = std::stof(ce3);
					cd.w = std::stof(ce4);
					cd.h = std::stof(ce5);
					cd.left_offset = std::stof(ce6);
					cd.right_offset = std::stof(ce7);
				}
				catch (std::exception const& e)
				{
					spdlog::error("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 中发现无法解析的行：'{}' ({})", path, line, e.what());
					return {};
				}

				cd.state = 1; // 成功

				return cd;
			};

			for (auto& line : lines)
			{
				if (line.empty())
				{
					// 跳过空白行
				}
				else if (line.starts_with("[HGEFONT]"))
				{
					// 跳过字体识别符
				}
				else if (line.starts_with("Bitmap="))
				{
					texture = line.substr(7);
				}
				else if (line.starts_with("Char="))
				{
					auto char_data = readCharData(line);
					if (!char_data.state)
					{
						continue;
					}
					if (m_map.find((uint32_t)char_data.code) != m_map.end())
					{
						spdlog::error("[luastg] 加载 HGE 纹理字体出错，在字体定义文件 '{}' 中发现重复的字符定义：{}", path, line);
						continue;
					}
					// 转换为字形数据
					Core::Graphics::GlyphInfo const glyph_info = {
						.texture_index = 0,
						.texture_rect = Core::RectF(
							char_data.x,
							char_data.y,
							char_data.x + char_data.w,
							char_data.y + char_data.h),
						.size = Core::Vector2F(char_data.w, char_data.h),
						.position = Core::Vector2F(char_data.left_offset, char_data.h),
						.advance = Core::Vector2F(char_data.w + char_data.left_offset + char_data.right_offset, 0),
					};
					m_map.emplace((uint32_t)char_data.code, glyph_info);
				}
				else
				{
					errorLineParser(line);
				}
			}

			return true;
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

			// 解析
			std::string_view font_define((char*)src.data(), src.size());
			std::string_view texture;
			if (!readDefine(path, font_define, texture))
			{
				throw std::runtime_error("hgeFont::hgeFont");
			}

			// 加载纹理
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
				std::filesystem::path wide_path(utf8::to_wstring(path));
				wide_path.remove_filename();
				wide_path /= utf8::to_wstring(texture);
				std::string texture_path(utf8::to_string(wide_path.wstring()));
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
		Core::Vector2F readVec2Str(std::string const& Str)
		{
			Core::Vector2F tRet;
			size_t const sep = Str.find_first_of(',');
			if (Str.empty() || sep == std::string::npos)
				throw std::runtime_error("invalid f2dTexturedFont Measure vec2");
			std::string n1 = Str.substr(0, sep);
			std::string n2 = Str.substr(sep + 1);
			tRet.x = std::stof(n1);
			tRet.y = std::stof(n2);
			return tRet;
		}
		void loadDefine(pugi::xml_document& doc)
		{
			float const u_scale = 1.0f / (float)m_texture->getSize().x;
			float const v_scale = 1.0f / (float)m_texture->getSize().y;

			pugi::xml_node root = doc.root().child("f2dTexturedFont");
			if (!root)
			{
				throw std::runtime_error("not a f2dTexturedFont");
			}

			pugi::xml_node measure = root.child("Measure");
			if (!measure)
			{
				throw std::runtime_error("invalid f2dTexturedFont Measure");
			}
			m_line_height = measure.attribute("LineHeight").as_float();
			m_ascender = measure.attribute("Ascender").as_float();
			m_descender = measure.attribute("Descender").as_float();

			pugi::xml_node char_list = root.child("CharList");
			if (!char_list)
			{
				throw std::runtime_error("invalid f2dTexturedFont CharList");
			}
			for (pugi::xml_node item = char_list.child("Item"); item; item = item.next_sibling("Item"))
			{
				std::string const c = item.attribute("Char").as_string();
				utf::utf8reader reader(c.c_str(), c.length());
				char32_t code = 0;
				if (!reader.step(code))
				{
					throw std::runtime_error("invalid char");
				}

				Core::Vector2F const Advance = readVec2Str(item.attribute("Advance").as_string());
				Core::Vector2F const BrushPos = readVec2Str(item.attribute("BrushPos").as_string());
				Core::Vector2F const GlyphSize = readVec2Str(item.attribute("Size").as_string());
				Core::Vector2F GlyphPosA = readVec2Str(item.attribute("Pos").as_string());
				Core::Vector2F GlyphPosB = GlyphPosA + readVec2Str(item.attribute("Size").as_string());
				GlyphPosA.x *= u_scale;
				GlyphPosB.x *= u_scale;
				GlyphPosA.y *= v_scale;
				GlyphPosB.y *= v_scale;

				Core::Graphics::GlyphInfo const glyph_info = {
					.texture_index = 0,
					.texture_rect = Core::RectF(GlyphPosA, GlyphPosB),
					.size = GlyphSize,
					.position = BrushPos,
					.advance = Advance,
				};
				m_map[(uint32_t)code] = glyph_info;
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
				std::filesystem::path wide_path(utf8::to_wstring(path));
				wide_path.remove_filename();
				wide_path /= utf8::to_wstring(raw_texture_path);
				std::string texture_path(utf8::to_string(wide_path.wstring()));
				if (!LAPP.GetAppModel()->getDevice()->createTextureFromFile(texture_path, mipmap, ~m_texture))
				{
					spdlog::error("[luastg] 加载 fancy2d 纹理字体失败，无法加载纹理 '{}'", texture_path);
					throw std::runtime_error("f2dFont::f2dFont");
				}
			}

			// 解析
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_buffer(src.data(), src.size());
			if (!result)
			{
				spdlog::error("[luastg] 加载 fancy2d 纹理字体失败，无法解析字体定义文件 '{}' ({})", path, result.description());
				throw std::runtime_error("f2dFont::f2dFont");
			}
			loadDefine(doc);
		}
		~f2dFont()
		{
		}
	};

	ResourceFontImpl::ResourceFontImpl(const char* name, std::string_view hge_path, bool mipmap)
		: ResourceBaseImpl(ResourceType::SpriteFont, name)
		, m_BlendMode(BlendMode::MulAlpha)
		, m_BlendColor(Core::Color4B(0xFFFFFFFFu))
	{
		m_glyphmgr.attach(new hgeFont(hge_path, mipmap));
	}
	ResourceFontImpl::ResourceFontImpl(const char* name, std::string_view f2d_path, std::string_view tex_path, bool mipmap)
		: ResourceBaseImpl(ResourceType::SpriteFont, name)
		, m_BlendMode(BlendMode::MulAlpha)
		, m_BlendColor(Core::Color4B(0xFFFFFFFFu))
	{
		m_glyphmgr.attach(new f2dFont(f2d_path, tex_path, mipmap));
	}
	ResourceFontImpl::ResourceFontImpl(const char* name, Core::Graphics::IGlyphManager* p_mgr)
		: ResourceBaseImpl(ResourceType::SpriteFont, name)
		, m_glyphmgr(p_mgr)
		, m_BlendMode(BlendMode::MulAlpha)
		, m_BlendColor(Core::Color4B(0xFFFFFFFFu))
	{
	}
}
