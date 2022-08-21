#include "GameResource/ResourceMgr.h"
#ifdef USING_DEAR_IMGUI
#include "imgui.h"
#endif

static std::string bytes_count_to_string(unsigned long long size)
{
	int count = 0;
	char buffer[64] = {};
	if (size < 1024) // B
	{
		count = std::snprintf(buffer, 64, "%u B", (unsigned int)size);
	}
	else if (size < (1024 * 1024)) // KB
	{
		count = std::snprintf(buffer, 64, "%.2f KB", (double)size / 1024.0);
	}
	else if (size < (1024 * 1024 * 1024)) // MB
	{
		count = std::snprintf(buffer, 64, "%.2f MB", (double)size / 1048576.0);
	}
	else // GB
	{
		count = std::snprintf(buffer, 64, "%.2f GB", (double)size / 1073741824.0);
	}
	return std::string(buffer, count);
}

namespace LuaSTGPlus
{
	void ResourceMgr::ShowResourceManagerDebugWindow(bool* p_open)
	{
	#ifdef USING_DEAR_IMGUI
		if (ImGui::Begin("Resource Manager##lstg.ResourceManager", p_open))
		{
			static int timer = 0;

			static int current_pool = 0;
			static char const* const pool_names[] = {
				"Global",
				"Stage",
			};
			ImGui::Combo("Resource Set", &current_pool, pool_names, 2);

			ResourcePool* p_pool = nullptr;
			switch (current_pool)
			{
			case 0:
				p_pool = &m_GlobalResourcePool;
				break;
			case 1:
				p_pool = &m_StageResourcePool;
				break;
			default:
				break;
			}

			auto draw_preview_scaling = [](float& scale) -> void
			{
				ImGui::PushID(&scale);
				ImGui::SliderFloat("##SliderFloat", &scale, 0.25f, 4.0f);
				ImGui::SameLine();
				if (ImGui::Button("Reset##Button"))
				{
					scale = 1.0f;
				}
				ImGui::SameLine();
				ImGui::Text("Preview Scaling");
				ImGui::PopID();
			};
			auto draw_texture0 = [](Core::Graphics::ITexture2D* p_tex, float scale) -> void
			{
				auto const size = p_tex->getSize();
				ImGui::Image(
					p_tex->getNativeHandle(),
					ImVec2(scale * (float)size.x, scale * (float)size.y),
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			};
			auto draw_texture = [](ResTexture* p_res, bool show_info, float scale) -> void
			{
				auto const size = p_res->GetTexture()->getSize();
				if (show_info)
				{
					ImGui::Text("Size: %u x %u", size.x, size.y);
					ImGui::Text("RenderTarget: %s", p_res->IsRenderTarget() ? "Yes" : "Not");
					ImGui::Text("Dynamic: %s", p_res->IsRenderTarget() ? "Yes" : "Not");
					unsigned long long mem_usage = size.x * size.y * 4;
					ImGui::Text("Adapter Memory Usage (Approximate): %s", bytes_count_to_string(mem_usage).c_str());
				}
				ImGui::Image(
					p_res->GetTexture()->getNativeHandle(),
					ImVec2(scale * (float)size.x, scale * (float)size.y),
					ImVec2(0.0f, 0.0f),
					ImVec2(1.0f, 1.0f),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			};
			auto draw_sprite = [](Core::Graphics::ISprite* p_res, bool show_info, bool focus, float scale) -> void {
				auto color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
				if (focus)
				{
					color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
				}
				auto* p_tex = p_res->getTexture();
				auto tex_size = p_tex->getSize();
				auto rc = p_res->getTextureRect();
				auto cp = p_res->getTextureCenter() - rc.a;
				if (show_info)
				{
					ImGui::Text("Pos: %.2f x %.2f", rc.a.x, rc.a.y);
					ImGui::Text("Size: %.2f x %.2f", rc.b.x - rc.a.x, rc.b.y - rc.a.y);
					ImGui::Text("Center: %.2f x %.2f", cp.x, cp.y);
					ImGui::Text("Units Per Pixel: %.4f", p_res->getUnitsPerPixel());
				}
				ImGui::Image(
					p_tex->getNativeHandle(),
					ImVec2(scale * (rc.b.x - rc.a.x), scale * (rc.b.y - rc.a.y)),
					ImVec2(rc.a.x / (float)tex_size.x, rc.a.y / (float)tex_size.y),
					ImVec2(rc.b.x / (float)tex_size.x, rc.b.y / (float)tex_size.y),
					ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
					color);
			};
			
			if (p_pool)
			{
				if (ImGui::BeginTabBar("##lstg.ResourceManager"))
				{
					if (ImGui::BeginTabItem("Texture"))
					{
						static unsigned long long total_texture_memory_usage = 0;

						ImGui::Text("Total Resources: %u", p_pool->m_TexturePool.size());
						ImGui::Text("Total Adapter Memory Usage (Approximate): %s", bytes_count_to_string(total_texture_memory_usage).c_str());

						total_texture_memory_usage = 0;

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_TexturePool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								auto* p_res = v.second->GetTexture();
								auto const p_tex_size = p_res->getSize();
								unsigned long long mem_usage = p_tex_size.x * p_tex_size.y * 4;
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									static float preview_scale = 1.0f;
									draw_preview_scaling(preview_scale);
									draw_texture(*(v.second), true, preview_scale);
									ImGui::TreePop();
								}
								res_i += 1;
								total_texture_memory_usage += mem_usage;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Sprite"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_SpritePool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_SpritePool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									static float preview_scale = 1.0f;
									draw_preview_scaling(preview_scale);
									draw_sprite(v.second->GetSprite(), true, false, preview_scale);
									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Sprite Sequence"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_AnimationPool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_AnimationPool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									static float preview_scale = 1.0f;
									draw_preview_scaling(preview_scale);
									ImGui::Text("Sprite Count: %u", v.second->GetCount());
									ImGui::Text("Animation Interval: %u", v.second->GetInterval());
									uint32_t ani_idx = v.second->GetSpriteIndexByTimer(timer);
									draw_sprite(v.second->GetSprite(ani_idx), false, false, preview_scale);
									static bool same_line = false;
									ImGui::Checkbox("Same Line Preview", &same_line);
									for (uint32_t img_idx = 0; img_idx < v.second->GetCount(); img_idx += 1)
									{
										if (same_line)
										{
											draw_sprite(v.second->GetSprite(img_idx), false, img_idx == ani_idx, preview_scale);
											if (img_idx < (v.second->GetCount() - 1))
												ImGui::SameLine();
										}
										else
										{
											if (ImGui::TreeNode(v.second->GetSprite(img_idx), "Sprite %u", img_idx))
											{
												draw_sprite(v.second->GetSprite(img_idx), true, false, preview_scale);
												ImGui::TreePop();
											}
										}
									}
									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Music"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_MusicPool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_MusicPool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Sound Effect"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_SoundSpritePool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_SoundSpritePool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Particle System"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_ParticlePool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_ParticlePool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Sprite Font"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_SpriteFontPool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_SpriteFontPool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									auto* mgr = v.second->GetGlyphManager();
									auto* p_tex0 = mgr->getTexture(0);

									ImGui::Text("Size: %u x %u (x %u)", p_tex0->getSize().x, p_tex0->getSize().y, mgr->getTextureCount());
									ImGui::Text("Dynamic: No");
									unsigned long long mem_usage = p_tex0->getSize().x * p_tex0->getSize().y * 4 * mgr->getTextureCount();
									ImGui::Text("Adapter Memory Usage (Approximate): %s", bytes_count_to_string(mem_usage).c_str());

									static float preview_scale = 1.0f;
									draw_preview_scaling(preview_scale);
									for (uint32_t tidx = 0; tidx < mgr->getTextureCount(); tidx += 1)
									{
										draw_texture0(mgr->getTexture(tidx), preview_scale);
									}

									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Vector Font"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_TTFFontPool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_TTFFontPool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									auto* mgr = v.second->GetGlyphManager();
									auto* p_tex0 = mgr->getTexture(0);

									ImGui::Text("Size: %u x %u (x %u)", p_tex0->getSize().x, p_tex0->getSize().y, mgr->getTextureCount());
									ImGui::Text("Dynamic: Yes");
									unsigned long long mem_usage = p_tex0->getSize().x * p_tex0->getSize().y * 4 * mgr->getTextureCount();
									ImGui::Text("Memory Usage (Approximate): %s", bytes_count_to_string(mem_usage).c_str());
									ImGui::Text("Adapter Memory Usage (Approximate): %s", bytes_count_to_string(mem_usage).c_str());

									static float preview_scale = 1.0f;
									draw_preview_scaling(preview_scale);
									for (uint32_t tidx = 0; tidx < mgr->getTextureCount(); tidx += 1)
									{
										draw_texture0(mgr->getTexture(tidx), preview_scale);
									}

									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Post Effect"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_FXPool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_FXPool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Model"))
					{
						ImGui::Text("Total Resources: %u", p_pool->m_ModelPool.size());

						static ImGuiTextFilter filter;
						filter.Draw();

						int res_i = 0;
						for (auto& v : p_pool->m_ModelPool)
						{
							if (filter.PassFilter(v.second->GetResName().c_str()))
							{
								if (ImGui::TreeNode(*v.second,
									"%d. %s",
									res_i,
									v.second->GetResName().c_str()
								))
								{
									ImGui::TreePop();
								}
								res_i += 1;
							}
						}

						ImGui::EndTabItem();
					}

					ImGui::EndTabBar();
				}
			}

			timer += 1;
		}
		ImGui::End();
	#endif
	}
}
