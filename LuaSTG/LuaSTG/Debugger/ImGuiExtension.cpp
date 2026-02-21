#include "ImGuiExtension.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <Xinput.h>
#include <psapi.h>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_freetype.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "implot.h"

#include "lua.hpp"
#include "lua_imgui.hpp"

#include "windows/XInput.hpp"
#include "core/Configuration.hpp"
#include "utf8.hpp"

#include "AppFrame.h"
#include "LuaBinding/LuaWrapper.hpp"

using std::string_view_literals::operator ""sv;

namespace {
	std::string toReadableDataSize(uint64_t const size) {
		int count{};
		char buffer[64]{};
		if (size < UINT64_C(1024)) {
			count = std::snprintf(buffer, 64, "%u B", static_cast<uint32_t>(size));
		}
		else if (size < UINT64_C(1024) * 1024) {
			count = std::snprintf(buffer, 64, "%.2f KiB", static_cast<double>(size) / 1024.0);
		}
		else if (size < UINT64_C(1024) * 1024 * 1024) {
			count = std::snprintf(buffer, 64, "%.2f MiB", static_cast<double>(size) / 1024.0 / 1024.0);
		}
		else {
			count = std::snprintf(buffer, 64, "%.2f GiB", static_cast<double>(size) / 1024.0 / 1024.0 / 1024.0);
		}
		return { buffer, static_cast<size_t>(count) };
	}
}

namespace imgui {
	class MemoryUsageWindow {
	public:
		void show(bool* is_open = nullptr) {
		#define format_size(x) (toReadableDataSize(x).c_str())
			if (is_open == nullptr || (is_open != nullptr && *is_open)) {
				if (m_want_fit_window_size) {
					m_want_fit_window_size = false;
					ImGui::SetNextWindowSize(ImVec2(), ImGuiCond_Always);
				}
				if (ImGui::Begin("Memory Usage", is_open)) {
					if (ImGui::Checkbox("More Informations", &m_more_details)) {
						m_want_fit_window_size = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("Auto Size")) {
						m_want_fit_window_size = true;
					}
					if (ImGui::CollapsingHeader("Global", ImGuiTreeNodeFlags_DefaultOpen)) {
						MEMORYSTATUSEX info = { sizeof(MEMORYSTATUSEX) };
						if (GlobalMemoryStatusEx(&info)) {
							if (m_more_details) ImGui::Text("MemoryLoad: %u%%", info.dwMemoryLoad);
							if (m_more_details) ImGui::Text("TotalPhys: %s", format_size(info.ullTotalPhys));
							if (m_more_details) ImGui::Text("UsedPhys: %s", format_size(info.ullTotalPhys - info.ullAvailPhys));
							if (m_more_details) ImGui::Text("AvailPhys: %s", format_size(info.ullAvailPhys));
							if (m_more_details) ImGui::Text("TotalPageFile: %s", format_size(info.ullTotalPageFile));
							if (m_more_details) ImGui::Text("UsedPageFile: %s", format_size(info.ullTotalPageFile - info.ullAvailPageFile));
							if (m_more_details) ImGui::Text("AvailPageFile: %s", format_size(info.ullAvailPageFile));
							if (m_more_details) ImGui::Text("TotalVirtual: %s", format_size(info.ullTotalVirtual));
							ImGui::Text("UsedVirtual: %s", format_size(info.ullTotalVirtual - info.ullAvailVirtual));
							if (m_more_details) ImGui::Text("AvailVirtual: %s", format_size(info.ullAvailVirtual));
							if (m_more_details) ImGui::Text("AvailExtendedVirtual: %s", format_size(info.ullAvailExtendedVirtual));
						}
					}
					if (ImGui::CollapsingHeader("Process", ImGuiTreeNodeFlags_DefaultOpen)) {
						HANDLE process = GetCurrentProcess();
						PROCESS_MEMORY_COUNTERS_EX2 info{};
						BOOL result{};
						if (m_process_memory_counters_v3) {
							info.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX2);
							result = GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&info), info.cb);
							if (!result) {
								m_process_memory_counters_v3 = false;
							}
						}
						else if (m_process_memory_counters_v2) {
							info.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);
							result = GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&info), info.cb);
							if (!result) {
								m_process_memory_counters_v2 = false;
							}
						}
						else {
							info.cb = sizeof(PROCESS_MEMORY_COUNTERS);
							result = GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&info), info.cb);
						}
						if (m_more_details) {
							if (m_process_memory_counters_v3) {
								ImGui::TextUnformatted("Current API Version: 3");
							}
							else if (m_process_memory_counters_v2) {
								ImGui::TextUnformatted("Current API Version: 2");
							}
							else {
								ImGui::TextUnformatted("Current API Version: 1");
							}
						}
						if (result) {
							if (m_more_details) ImGui::Text("PageFaultCount: %u", info.PageFaultCount);
							ImGui::Text("PeakWorkingSetSize: %s", format_size(info.PeakWorkingSetSize));
							ImGui::Text("WorkingSetSize: %s", format_size(info.WorkingSetSize));
							if (m_more_details) ImGui::Text("QuotaPeakPagedPoolUsage: %s", format_size(info.QuotaPeakPagedPoolUsage));
							if (m_more_details) ImGui::Text("QuotaPagedPoolUsage: %s", format_size(info.QuotaPagedPoolUsage));
							if (m_more_details) ImGui::Text("QuotaPeakNonPagedPoolUsage: %s", format_size(info.QuotaPeakNonPagedPoolUsage));
							if (m_more_details) ImGui::Text("QuotaNonPagedPoolUsage: %s", format_size(info.QuotaNonPagedPoolUsage));
							ImGui::Text("PagefileUsage: %s", format_size(info.PagefileUsage));
							ImGui::Text("PeakPagefileUsage: %s", format_size(info.PeakPagefileUsage));
							ImGui::Text("PrivateUsage: %s", format_size(info.PrivateUsage));
							ImGui::Text("PrivateWorkingSetSize: %s", format_size(info.PrivateWorkingSetSize));
							ImGui::Text("SharedCommitUsage: %s", format_size(info.SharedCommitUsage));
						}
					}
					if (ImGui::CollapsingHeader("Lua", ImGuiTreeNodeFlags_DefaultOpen)) {
						lua_State* L = LAPP.GetLuaEngine();
						auto const vm_kb = lua_gc(L, LUA_GCCOUNT, 0);
						auto const vm_b = lua_gc(L, LUA_GCCOUNTB, 0);
						auto const size = (DWORDLONG)vm_kb * (DWORDLONG)1024 + (DWORDLONG)vm_b;
						ImGui::Text("Lua Virtual Machine: %s", format_size(size));
					}
					if (ImGui::CollapsingHeader("Graphics", ImGuiTreeNodeFlags_DefaultOpen)) {
						auto const info = LAPP.getGraphicsDevice()->getMemoryStatistics();
						if (m_more_details) ImGui::Text("Local Budget: %s", format_size(info.local.budget));
						ImGui::Text("Local Current Usage: %s", format_size(info.local.current_usage));
						if (m_more_details) ImGui::Text("Local Available For Reservation: %s", format_size(info.local.available_for_reservation));
						if (m_more_details) ImGui::Text("Local Current Reservation: %s", format_size(info.local.current_reservation));
						if (m_more_details) ImGui::Text("Non-Local Budget: %s", format_size(info.non_local.budget));
						ImGui::Text("Non-Local Current Usage: %s", format_size(info.non_local.current_usage));
						if (m_more_details) ImGui::Text("Non-Local Available For Reservation: %s", format_size(info.non_local.available_for_reservation));
						if (m_more_details) ImGui::Text("Non-Local Current Reservation: %s", format_size(info.non_local.current_reservation));
					}
				}
				ImGui::End();
			}
		#undef format_size
		}
	
		static MemoryUsageWindow& getInstance() {
			static MemoryUsageWindow instance;
			return instance;
		}

	private:
		MemoryUsageWindow() = default;

		bool m_want_fit_window_size{ true };
		bool m_more_details{ true };
		bool m_process_memory_counters_v3{ true };
		bool m_process_memory_counters_v2{ true };
	};
}

// lua imgui backend binding

namespace {
	void showParticleSystemEditor(bool* p_open, luastg::IParticlePool* sys) {
		struct EditorData {
			bool bContinuous = false;
			bool bSyncParticleLife = false;
			bool bSyncSpeed = false;
			bool bSyncGravity = false;
			bool bSyncRadialAccel = false;
			bool bSyncTangentialAccel = false;
			bool bSyncSize = false;
			bool bSyncSpin = false;
			bool bSyncAlpha = false;
			int nBlendMode = 0;
		};
		static std::unordered_map<void*, EditorData> g_EditorData;
		if (ImGui::Begin("Particle System Editor", p_open)) {
			if (sys) {
				auto& info = sys->GetParticleSystemInfo();
				if (g_EditorData.find(&info) == g_EditorData.end()) {
					g_EditorData.emplace(&info, EditorData{
						.bContinuous = (info.fLifetime - (-1.0f)) < std::numeric_limits<float>::min(),
						.bSyncParticleLife = false,
						.bSyncSpeed = false,
						.bSyncGravity = false,
						.bSyncRadialAccel = false,
						.bSyncTangentialAccel = false,
						.bSyncSize = false,
						.bSyncSpin = false,
						.bSyncAlpha = false,
						.nBlendMode = sys->GetBlendMode() == luastg::BlendMode::MulAdd ? 0 : 1,
										 });
				}
				auto& data = g_EditorData[&info];

				auto widgetSyncMinMax = [](std::string_view name, float& minv, float& maxv, float a, float b, bool& sync) -> void {
					std::string checkbox_text = "Sync##"; checkbox_text.append(name);
					std::string min_text = "Min##"; min_text.append(name);
					std::string max_text = "Max##"; max_text.append(name);
					ImGui::AlignTextToFramePadding();
					ImGui::Text(name.data());
					ImGui::SameLine();
					if (ImGui::Checkbox(checkbox_text.data(), &sync)) {
						if (sync) {
							maxv = minv;
						}
					}
					if (sync) {
						if (ImGui::SliderFloat(min_text.data(), &minv, a, b)) {
							maxv = minv;
						}
						if (ImGui::SliderFloat(max_text.data(), &maxv, a, b)) {
							minv = maxv;
						}
					}
					else {
						ImGui::SliderFloat(min_text.data(), &minv, a, b);
						ImGui::SliderFloat(max_text.data(), &maxv, a, b);
					}
				};

				auto widgetSyncStartEndVar = [](std::string_view name, float& begv, float& endv, float& varv, float a, float b, bool& sync) -> void {
					std::string checkbox_text = "Sync##"; checkbox_text.append(name);
					std::string beg_text = "Start##"; beg_text.append(name);
					std::string end_text = "End##"; end_text.append(name);
					std::string var_text = "Variation##"; var_text.append(name);
					ImGui::AlignTextToFramePadding();
					ImGui::Text(name.data());
					ImGui::SameLine();
					if (ImGui::Checkbox(checkbox_text.data(), &sync)) {
						if (sync) {
							endv = begv;
						}
					}
					if (sync) {
						if (ImGui::SliderFloat(beg_text.data(), &begv, a, b)) {
							endv = begv;
						}
						if (ImGui::SliderFloat(end_text.data(), &endv, a, b)) {
							begv = endv;
						}
					}
					else {
						ImGui::SliderFloat(beg_text.data(), &begv, a, b);
						ImGui::SliderFloat(end_text.data(), &endv, a, b);
					}
					ImGui::SliderFloat(var_text.data(), &varv, 0.0f, 1.0f);
				};

				if (ImGui::CollapsingHeader("System Information")) {
					ImGui::LabelText("Particle Alive", "%u", (uint32_t)sys->GetAliveCount());
					ImGui::LabelText("FPS", "%.2f", LAPP.GetFPS());
				}

				if (ImGui::CollapsingHeader("System Parameter")) {
					if (!data.bContinuous) {
						if (info.fLifetime < 0.0f) {
							info.fLifetime = 5.0f;
						}
						ImGui::SliderFloat("System Lifetime", &info.fLifetime, 0.0f, 10.0f);
					}
					else {
						float fFake = 5.0f;
						ImGui::SliderFloat("System Lifetime", &fFake, 0.0f, 10.0f);
					}
					ImGui::Checkbox("Continuous", &data.bContinuous);
					if (data.bContinuous) {
						info.fLifetime = -1.0f;
					}
					ImGui::Separator();

					ImGui::SliderInt("Emission", &info.nEmission, 0, 1000, "%d (p/sec)");
					ImGui::Separator();

					widgetSyncMinMax("Particle Lifetime", info.fParticleLifeMin, info.fParticleLifeMax, 0.0f, 5.0f, data.bSyncParticleLife);
					ImGui::Separator();

					char const* const chBlendMode[2] = {
						"Add",
						"Alpha",
					};
					if (ImGui::Combo("Blend Mode", &data.nBlendMode, chBlendMode, 2)) {
						sys->SetBlendMode(data.nBlendMode == 0 ? luastg::BlendMode::MulAdd : luastg::BlendMode::MulAlpha);
					}
				}

				if (ImGui::CollapsingHeader("Particle Movement")) {
					if (info.bRelative) {
						float fDir = info.fDirection;
						ImGui::SliderAngle("Direction", &fDir, 0.0f, 360.0f);
					}
					else {
						ImGui::SliderAngle("Direction", &info.fDirection, 0.0f, 360.0f);
					}
					ImGui::Checkbox("Relative", &info.bRelative);
					ImGui::Separator();

					ImGui::SliderAngle("Spread", &info.fSpread, 0.0f, 360.0f);
					ImGui::Separator();

					widgetSyncMinMax("Start Speed", info.fSpeedMin, info.fSpeedMax, -300.0f, 300.0f, data.bSyncSpeed);
					ImGui::Separator();

					widgetSyncMinMax("Gravity", info.fGravityMin, info.fGravityMax, -900.0f, 900.0f, data.bSyncGravity);
					ImGui::Separator();

					widgetSyncMinMax("Radial Acceleration", info.fRadialAccelMin, info.fRadialAccelMax, -900.0f, 900.0f, data.bSyncRadialAccel);
					ImGui::Separator();

					widgetSyncMinMax("Tangential Acceleration", info.fTangentialAccelMin, info.fTangentialAccelMax, -900.0f, 900.0f, data.bSyncGravity);
				}

				if (ImGui::CollapsingHeader("Particle Appearance")) {
					widgetSyncStartEndVar("Particle Size", info.fSizeStart, info.fSizeEnd, info.fSizeVar, 1.0f / 32.0f, 100.0f / 32.0f, data.bSyncSize);
					ImGui::Separator();

					widgetSyncStartEndVar("Particle Spin", info.fSpinStart, info.fSpinEnd, info.fSpinVar, -50.0f, 50.0f, data.bSyncSpin);
					ImGui::Separator();

					widgetSyncStartEndVar("Particle Alpha", info.colColorStart[3], info.colColorEnd[3], info.fAlphaVar, 0.0f, 1.0f, data.bSyncAlpha);
					ImGui::Separator();

					ImGui::AlignTextToFramePadding();
					ImGui::Text("Particle Color");
					ImGui::ColorEdit3("Start##Particle Color", info.colColorStart);
					ImGui::ColorEdit3("End##Particle Color", info.colColorEnd);
					ImGui::SliderFloat("Variation##Particle Color", &info.fColorVar, 0.0f, 1.0f);
				}
			}
		}
		ImGui::End();
	}

	int lib_NewFrame(lua_State* L) {
		bool const allow_set_cursor = lua_toboolean(L, 1);
		imgui::updateEngine(allow_set_cursor);
		return 0;
	}
	int lib_RenderDrawData(lua_State* L) {
		std::ignore = L;
		imgui::drawEngine();
		return 0;
	}
	// Remove it
	int lib_CacheGlyphFromString(lua_State*) {
		return 0;
	}

	int lib_ShowTestInputWindow(lua_State* L) {
		if (lua_gettop(L) >= 1) {
			bool v = lua_toboolean(L, 1);
			imgui::showTestInputWindow(&v);
			lua_pushboolean(L, v);
			return 1;
		}
		else {
			imgui::showTestInputWindow();
			return 0;
		}
	}
	int lib_ShowMemoryUsageWindow(lua_State* L) {
		bool v = (lua_gettop(L) >= 1) ? lua_toboolean(L, 1) : true;
		imgui::MemoryUsageWindow::getInstance().show(&v);
		lua_pushboolean(L, v);
		return 1;
	}
	int lib_ShowFrameStatistics(lua_State* L) {
		constexpr size_t arr_size = 3600;
		static bool is_init = false;
		static std::vector<double> arr_x;
		static std::vector<double> arr_wait_time;
		static std::vector<double> arr_update_time;
		static std::vector<double> arr_render_time;
		static std::vector<double> arr_present_time;
		static std::vector<double> arr_total_time;
		static std::vector<double> arr_mem_mem;
		static std::vector<double> arr_mem_gpu;
		static std::vector<double> arr_mem_lua;
		static std::vector<double> arr_obj_alloc;
		static std::vector<double> arr_obj_free;
		static std::vector<double> arr_obj_alive;
		static std::vector<double> arr_obj_colli;
		static std::vector<double> arr_obj_colli_cb;
		static std::vector<double> arr_gpu_render_time;
		static size_t arr_index = 0;
		static size_t record_range = 240;
		constexpr size_t record_range_min = 60;
		constexpr size_t record_range_max = 3600;
		static float height = 384.0f;
		static float height_gpu = 384.0f;
		static float height_2 = 384.0f;
		static bool auto_fit = true;
		static bool auto_fit_gpu = true;
		static bool auto_fit_2 = true;

		bool v = (lua_gettop(L) >= 1) ? lua_toboolean(L, 1) : true;
		if (v) {
			if (ImGui::Begin("Frame Statistics", &v)) {
				// data buffer

				if (!is_init) {
					is_init = true;

					arr_x.resize(arr_size);
					for (size_t x = 0; x < arr_size; x += 1) {
						arr_x[x] = (double)x;
					}

					arr_wait_time.resize(arr_size);
					arr_update_time.resize(arr_size);
					arr_render_time.resize(arr_size);
					arr_present_time.resize(arr_size);
					arr_total_time.resize(arr_size);

					arr_mem_mem.resize(arr_size);
					arr_mem_gpu.resize(arr_size);
					arr_mem_lua.resize(arr_size);

					arr_obj_alloc.resize(arr_size);
					arr_obj_free.resize(arr_size);
					arr_obj_alive.resize(arr_size);
					arr_obj_colli.resize(arr_size);
					arr_obj_colli_cb.resize(arr_size);

					arr_gpu_render_time.resize(arr_size);
				}

				ImGui::SliderScalar("Record Range", sizeof(size_t) == 8 ? ImGuiDataType_U64 : ImGuiDataType_U32, &record_range, &record_range_min, &record_range_max);
				record_range = std::clamp<size_t>(record_range, 2, record_range_max);

				// frame time

				if (ImGui::CollapsingHeader("Frame Time")) {
					auto info = LAPP.getFrameStatistics();

					ImGui::Text("Wait   : %.3fms", info.wait_time * 1000.0);
					ImGui::Text("Update : %.3fms", info.update_time * 1000.0);
					ImGui::Text("Render : %.3fms", info.render_time * 1000.0);
					ImGui::Text("Present: %.3fms", info.present_time * 1000.0);
					ImGui::Text("Total  : %.3fms", info.total_time * 1000.0);

					ImGui::SliderFloat("Timeline Height", &height, 256.0f, 512.0f);
					ImGui::Checkbox("Auto-Fit Y Axis", &auto_fit);

					arr_update_time[arr_index] = 1000.0 * (info.update_time);
					arr_render_time[arr_index] = 1000.0 * (info.update_time + info.render_time);
					arr_present_time[arr_index] = 1000.0 * (info.update_time + info.render_time + info.present_time);
					arr_wait_time[arr_index] = 1000.0 * (info.update_time + info.render_time + info.present_time + info.wait_time);
					arr_total_time[arr_index] = 1000.0 * (info.total_time);

					if (ImPlot::BeginPlot("##Frame Statistics", ImVec2(-1, height), 0)) {
						//ImPlot::SetupAxes("Frame", "Time", flags, flags);
						ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)(record_range - 1), ImGuiCond_Always);
						//ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1000.0 / 18.0, ImGuiCond_Always);
						if (auto_fit)
							ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
						else
							ImPlot::SetupAxes(NULL, NULL);

						ImPlot::SetupLegend(ImPlotLocation_North, ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);

						static double arr_ms[] = {
							1000.0 / 60.0,
							1000.0 / 30.0,
							1000.0 / 20.0,
						};
						ImPlot::SetNextLineStyle(ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
						ImPlot::PlotInfLines("##60 FPS", arr_ms, 1, ImPlotInfLinesFlags_Horizontal);
						//ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.2f, 0.2f, 1.0f));
						//ImPlot::PlotHLines("##30 FPS", arr_ms + 1, 1);
						//ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
						//ImPlot::PlotHLines("##20 FPS", arr_ms + 2, 1);

						ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
						ImPlot::PlotShaded("Total", arr_x.data(), arr_wait_time.data(), arr_total_time.data(), (int)record_range);
						ImPlot::PlotShaded("Wait", arr_x.data(), arr_present_time.data(), arr_wait_time.data(), (int)record_range);
						ImPlot::PopStyleVar();
						ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.5f);
						ImPlot::PlotShaded("Present", arr_x.data(), arr_render_time.data(), arr_present_time.data(), (int)record_range);
						ImPlot::PlotShaded("Render", arr_x.data(), arr_update_time.data(), arr_render_time.data(), (int)record_range);
						ImPlot::PlotShaded("Update", arr_x.data(), arr_update_time.data(), (int)record_range);
						ImPlot::PopStyleVar();

						ImPlot::PlotLine("Total", arr_total_time.data(), (int)record_range);
						ImPlot::PlotLine("Wait", arr_wait_time.data(), (int)record_range);
						ImPlot::PlotLine("Present", arr_present_time.data(), (int)record_range);
						ImPlot::PlotLine("Render", arr_render_time.data(), (int)record_range);
						ImPlot::PlotLine("Update", arr_update_time.data(), (int)record_range);

						ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
						ImPlot::PlotInfLines("##Current Time", &arr_index, 1, ImPlotInfLinesFlags_None);

						ImPlot::EndPlot();
					}
				}

				// gpu time

				if (ImGui::CollapsingHeader("GPU Time")) {
					auto info = LAPP.getFrameRenderStatistics();

					ImGui::Text("Render : %.3fms", info.render_time * 1000.0);

					ImGui::SliderFloat("Timeline Height##GPU Time", &height_gpu, 256.0f, 512.0f);
					ImGui::Checkbox("Auto-Fit Y Axis##GPU Time", &auto_fit_gpu);

					// 还得再往前一帧
					arr_gpu_render_time[(arr_index + record_range - 1) % record_range] = 1000.0 * (info.render_time);

					if (ImPlot::BeginPlot("##Frame Render Statistics", ImVec2(-1, height_gpu), 0)) {
						//ImPlot::SetupAxes("Frame", "Time", flags, flags);
						ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)(record_range - 1), ImGuiCond_Always);
						//ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1000.0 / 18.0, ImGuiCond_Always);
						if (auto_fit_gpu)
							ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
						else
							ImPlot::SetupAxes(NULL, NULL);

						ImPlot::SetupLegend(ImPlotLocation_North, ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);

						static double arr_ms[] = {
							1000.0 / 60.0,
							1000.0 / 30.0,
							1000.0 / 20.0,
						};
						ImPlot::SetNextLineStyle(ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
						ImPlot::PlotInfLines("##60 FPS", arr_ms, 1, ImPlotInfLinesFlags_Horizontal);
						//ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.2f, 0.2f, 1.0f));
						//ImPlot::PlotHLines("##30 FPS", arr_ms + 1, 1);
						//ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
						//ImPlot::PlotHLines("##20 FPS", arr_ms + 2, 1);

						ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.5f);
						ImPlot::PlotShaded("Render", arr_x.data(), arr_gpu_render_time.data(), arr_gpu_render_time.data(), (int)record_range);
						ImPlot::PopStyleVar();

						ImPlot::PlotLine("Render", arr_gpu_render_time.data(), (int)record_range);

						ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
						ImPlot::PlotInfLines("##Current Time", &arr_index, 1, ImPlotInfLinesFlags_None);

						ImPlot::EndPlot();
					}
				}

				// memory

				if (ImGui::CollapsingHeader("Memory Usage")) {
					MEMORYSTATUSEX mem_info = { sizeof(MEMORYSTATUSEX) };
					GlobalMemoryStatusEx(&mem_info);
					auto gpu_info = LAPP.getGraphicsDevice()->getMemoryStatistics();
					lua_State* L_ = LAPP.GetLuaEngine();
					int lua_infokb = lua_gc(L_, LUA_GCCOUNT, 0);
					int lua_infob = lua_gc(L_, LUA_GCCOUNTB, 0);
					DWORDLONG lua_info = (DWORDLONG)lua_infokb * (DWORDLONG)1024 + (DWORDLONG)lua_infob;

					ImGui::Text("Avalid User Mode Memory Space: %s", toReadableDataSize(mem_info.ullAvailVirtual).c_str());
					ImGui::Text("User Mode Memory Space Usage: %s", toReadableDataSize(mem_info.ullTotalVirtual - mem_info.ullAvailVirtual).c_str());
					ImGui::Text("Lua Runtime Memory Usage: %s", toReadableDataSize(lua_info).c_str());
					ImGui::Text("Adapter Local Usage: %s", toReadableDataSize(gpu_info.local.current_usage).c_str());
					ImGui::Text("Adapter Non-Local Usage: %s", toReadableDataSize(gpu_info.non_local.current_usage).c_str());

					static float time_line_height = 384.0f;
					static bool time_line_auto_fit = true;
					ImGui::SliderFloat("Timeline Height##Memory Usage", &time_line_height, 256.0f, 512.0f);
					ImGui::Checkbox("Auto-Fit Y Axis##Memory Usage", &time_line_auto_fit);

					constexpr double const byte_to_MiB = 1.0 / (1024.0 * 1024.0);
					arr_mem_mem[arr_index] = (double)(mem_info.ullTotalVirtual - mem_info.ullAvailVirtual) * byte_to_MiB;
					arr_mem_gpu[arr_index] = (double)(gpu_info.local.current_usage + gpu_info.non_local.current_usage) * byte_to_MiB;
					arr_mem_lua[arr_index] = (double)lua_info * byte_to_MiB;

					if (ImPlot::BeginPlot("##Memory Usage Statistics", ImVec2(-1, time_line_height), 0)) {
						//ImPlot::SetupAxes("Frame", "Time", flags, flags);
						ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)(record_range - 1), ImGuiCond_Always);
						//ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1000.0 / 18.0, ImGuiCond_Always);
						if (time_line_auto_fit)
							ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
						else
							ImPlot::SetupAxes(NULL, NULL);

						ImPlot::SetupLegend(ImPlotLocation_North, ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);

						ImPlot::PlotLine("Memory (MiB)", arr_mem_mem.data(), (int)record_range);
						ImPlot::PlotLine("GPU (MiB)", arr_mem_gpu.data(), (int)record_range);
						ImPlot::PlotLine("Lua (MiB)", arr_mem_lua.data(), (int)record_range);

						ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
						ImPlot::PlotInfLines("##Current Time", &arr_index, 1, ImPlotInfLinesFlags_None);

						ImPlot::EndPlot();
					}
				}

				// object

				if (ImGui::CollapsingHeader("GameObject")) {
					auto obj_info = LAPP.GetGameObjectPool().DebugGetFrameStatistics();

					ImGui::Text("Create : %llu", obj_info.object_alloc);
					ImGui::Text("Return : %llu", obj_info.object_free);
					ImGui::Text("Active : %llu", obj_info.object_alive);
					ImGui::Text("Colli Check : %llu", obj_info.object_colli_check);
					ImGui::Text("Colli Callback : %llu", obj_info.object_colli_callback);

					ImGui::SliderFloat("Timeline Height##GameObject", &height_2, 256.0f, 512.0f);
					ImGui::Checkbox("Auto-Fit Y Axis##GameObject", &auto_fit_2);

					arr_obj_alloc[arr_index] = (double)obj_info.object_alloc;
					arr_obj_free[arr_index] = (double)obj_info.object_free;
					arr_obj_alive[arr_index] = (double)obj_info.object_alive;
					arr_obj_colli[arr_index] = (double)obj_info.object_colli_check;
					arr_obj_colli_cb[arr_index] = (double)obj_info.object_colli_callback;

					if (ImPlot::BeginPlot("##GameObject Statistics", ImVec2(-1, height_2), 0)) {
						//ImPlot::SetupAxes("Frame", "Time", flags, flags);
						ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)(record_range - 1), ImGuiCond_Always);
						//ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1000.0 / 18.0, ImGuiCond_Always);
						if (auto_fit_2)
							ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit);
						else
							ImPlot::SetupAxes(NULL, NULL);

						ImPlot::SetupLegend(ImPlotLocation_North, ImPlotLegendFlags_Horizontal | ImPlotLegendFlags_Outside);

						static double arr_ms[] = {
							1000.0 / 60.0,
							1000.0 / 30.0,
							1000.0 / 20.0,
						};
						//ImPlot::SetNextLineStyle(ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
						//ImPlot::PlotHLines("##60 FPS", arr_ms, 1);
						//ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.2f, 0.2f, 1.0f));
						//ImPlot::PlotHLines("##30 FPS", arr_ms + 1, 1);
						//ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
						//ImPlot::PlotHLines("##20 FPS", arr_ms + 2, 1);

						ImPlot::PlotLine("Create", arr_obj_alloc.data(), (int)record_range);
						ImPlot::PlotLine("Return", arr_obj_free.data(), (int)record_range);
						ImPlot::PlotLine("Active", arr_obj_alive.data(), (int)record_range);
						ImPlot::PlotLine("Colli Check", arr_obj_colli.data(), (int)record_range);
						ImPlot::PlotLine("Colli Callback", arr_obj_colli_cb.data(), (int)record_range);

						ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
						ImPlot::PlotInfLines("##Current Time", &arr_index, 1, ImPlotInfLinesFlags_None);

						ImPlot::EndPlot();
					}

				}

				// move next

				arr_index = (arr_index + 1) % record_range;
			}
			ImGui::End();
		}
		lua_pushboolean(L, v);
		return 1;
	}
	int lib_ShowResourceManagerDebugWindow(lua_State* L) {
		if (lua_gettop(L) >= 1) {
			bool v = lua_toboolean(L, 1);
			LRES.ShowResourceManagerDebugWindow(&v);
			lua_pushboolean(L, v);
			return 1;
		}
		else {
			LRES.ShowResourceManagerDebugWindow();
			return 0;
		}
	}
	int lib_ShowParticleSystemEditor(lua_State* L) {
		if (lua_gettop(L) >= 2) {
			bool v = lua_toboolean(L, 1);
			auto p = luastg::binding::ParticleSystem::Cast(L, 2);
			showParticleSystemEditor(&v, p->ptr);
			lua_pushboolean(L, v);
			return 1;
		}
		else {
			auto p = luastg::binding::ParticleSystem::Cast(L, 1);
			showParticleSystemEditor(nullptr, p->ptr);
			return 0;
		}
	}

	void imgui_binding_lua_register_backend(lua_State* L) {
		const luaL_Reg lib_fun[] = {
			{"NewFrame", &lib_NewFrame},
			{"RenderDrawData", &lib_RenderDrawData},
			{"CacheGlyphFromString", &lib_CacheGlyphFromString},
			{"ShowTestInputWindow", &lib_ShowTestInputWindow},
			{"ShowMemoryUsageWindow", &lib_ShowMemoryUsageWindow},
			{"ShowFrameStatistics", &lib_ShowFrameStatistics},
			{"ShowResourceManagerDebugWindow", &lib_ShowResourceManagerDebugWindow},
			{"ShowParticleSystemEditor", &lib_ShowParticleSystemEditor},
			{NULL, NULL},
		};
		const auto lib_func = (sizeof(lib_fun) / sizeof(luaL_Reg)) - 1;

		//                                      // ? m
		lua_pushstring(L, "backend");           // ? m k
		lua_createtable(L, 0, lib_func);        // ? m k t
		luaL_setfuncs(L, lib_fun, 0);           // ? m k t
		lua_settable(L, -3);                    // ? m
	}
}

// imgui backend binding

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
	bool g_imgui_initialized = false;
	bool g_imgui_impl_dx11_initialized = false;
}

namespace imgui {
	class ImGuiBackendEventListener
		: public core::IGraphicsDeviceEventListener
		, public core::IWindowEventListener {
	public:
		// IGraphicsDeviceEventListener

		void onGraphicsDeviceDestroy() override {
			g_imgui_impl_dx11_initialized = false;
			ImGui_ImplDX11_Shutdown();
		}
		void onGraphicsDeviceCreate() override {
			g_imgui_impl_dx11_initialized = false;
			auto const device = static_cast<ID3D11Device*>(LAPP.getGraphicsDevice()->getNativeHandle());
			ID3D11DeviceContext* context{};
			device->GetImmediateContext(&context);
			ImGui_ImplDX11_Init(device, context);
			context->Release();
		}

		// IWindowEventListener

		void onWindowCreate() override {
			ImGui_ImplWin32_Init(LAPP.getWindow()->getNativeHandle());
		}
		void onWindowDestroy() override {
			ImGui_ImplWin32_Shutdown();
			m_dpi_changed.store(0);
		}
		void onWindowDpiChange() override {
			m_dpi_changed.fetch_or(0x1);
		}
		NativeWindowMessageResult onNativeWindowMessage(void* const window, uint32_t const message, uintptr_t const arg1, intptr_t const arg2) override {
			if (auto const result = ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(window), message, arg1, arg2))
				return { result, true };
			return {};
		}

		// ImGuiBackendEventListener

		[[nodiscard]] bool isDpiChanged(bool const zero = false) {
			if (zero) {
				return 0x1 == m_dpi_changed.exchange(0);
			}
			return 0x1 == m_dpi_changed.load();
		}

	private:
		std::atomic_int m_dpi_changed;
	};
}

namespace {
	imgui::ImGuiBackendEventListener g_imgui_backend_event_listener;

	std::string const& getIniPath() {
		static bool g_ini_path_initialized = false;
		static std::string g_ini_path;

		if (!g_ini_path_initialized) {
			auto const& config = core::ConfigurationLoader::getInstance().getFileSystem();
			if (config.hasUser()) {
				std::filesystem::path directory;
				core::ConfigurationLoader::resolvePathWithPredefinedVariables(config.getUser(), directory, true);
				std::filesystem::path const path = directory / u8"imgui.ini"sv;
				auto const u8 = path.generic_u8string();
				g_ini_path = std::string_view(reinterpret_cast<char const*>(u8.data()), u8.size());
			}
			else {
				g_ini_path = "imgui.ini"sv;
			}
			g_ini_path_initialized = true;
		}

		return g_ini_path;
	}
	void addSystemFont() {
		constexpr std::array system_fonts{
			R"(C:\Windows\Fonts\msyh.ttc)"sv,
			R"(C:\Windows\Fonts\msyh.ttf)"sv, // Windows 7
		};
		std::error_code ec;
		for (auto const& system_font : system_fonts) {
			std::u8string_view const path{reinterpret_cast<char8_t const*>(system_font.data()), system_font.size()};
			if (std::filesystem::is_regular_file(path, ec)) {
				if (ImGui::GetIO().Fonts->AddFontFromFileTTF(system_font.data(), 16.0f)) {
					break;
				}
			}
		}
	}
	void applyStyle() {
		ImGuiStyle style;
		ImGui::StyleColorsDark(&style); // apply default dark theme colors

		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.PopupRounding = 0.0f;
		style.FrameRounding = 0.0f;
		style.ScrollbarRounding = 0.0f;
		style.GrabRounding = 0.0f;
		style.TabRounding = 0.0f;

		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 1.0f;
		style.TabBorderSize = 1.0f;

		style.FontScaleDpi = LAPP.getWindow()->getDPIScaling();
		style.ScaleAllSizes(LAPP.getWindow()->getDPIScaling());

		ImGui::GetStyle() = style;
	}
}

namespace imgui {
	void bindEngine() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = getIniPath().c_str(); // managed by imgui
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

		addSystemFont();
		applyStyle();

		g_imgui_backend_event_listener.onWindowCreate();
		auto const window = LAPP.getWindow();
		window->addEventListener(&g_imgui_backend_event_listener);

		g_imgui_backend_event_listener.onGraphicsDeviceCreate();
		auto const device = LAPP.getGraphicsDevice();
		device->addEventListener(&g_imgui_backend_event_listener);

		auto const vm = LAPP.GetLuaEngine();
		luaopen_imgui(vm);
		imgui_binding_lua_register_backend(vm);
		lua_pop(vm, 1);

		g_imgui_initialized = true;
	}
	void unbindEngine() {
		if (const auto device = LAPP.getGraphicsDevice(); device != nullptr) {
			device->removeEventListener(&g_imgui_backend_event_listener);
			g_imgui_backend_event_listener.onGraphicsDeviceDestroy();
		}
		else {
			g_imgui_backend_event_listener.onGraphicsDeviceDestroy();
		}

		if (const auto window = LAPP.getWindow(); window != nullptr) {
			window->removeEventListener(&g_imgui_backend_event_listener);
			g_imgui_backend_event_listener.onWindowDestroy();
		}
		else {
			g_imgui_backend_event_listener.onWindowDestroy();
		}

		ImPlot::DestroyContext();
		ImGui::DestroyContext();

		g_imgui_initialized = false;
		g_imgui_impl_dx11_initialized = false;
	}

	void cancelSetCursor() {
		if (g_imgui_initialized) {
			auto& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		}
	}
	void updateEngine(bool const allow_set_cursor) {
		tracy_zone_scoped_with_name("imgui.backend.NewFrame");
		if (g_imgui_initialized) {
			if (g_imgui_backend_event_listener.isDpiChanged(true)) {
				applyStyle();
			}

			if (allow_set_cursor) {
				constexpr int mask = ~static_cast<ImGuiConfigFlags>(ImGuiConfigFlags_NoMouseCursorChange);
				ImGui::GetIO().ConfigFlags &= mask;
			}
			
			{
				tracy_zone_scoped_with_name("imgui.backend.NewFrame-D3D11");
				ImGui_ImplDX11_NewFrame();
			}

			{
				tracy_zone_scoped_with_name("imgui.backend.NewFrame-WIN32");
				auto const ws = LAPP.getSwapChain()->getCanvasSize();
				auto const mt = LAPP.GetMousePositionTransformF();
				ImGui_ImplWin32_FrameOverride dt{};
				dt.DisplaySize.x = static_cast<float>(ws.x);
				dt.DisplaySize.y = static_cast<float>(ws.y);
				dt.MouseOffset.x = mt.x;
				dt.MouseOffset.y = mt.y;
				dt.MouseScale.x = mt.z;
				dt.MouseScale.y = mt.w;
				ImGui_ImplWin32_NewFrame(&dt);
			}

			g_imgui_impl_dx11_initialized = true;
			if (ImGui::GetIO().WantCaptureKeyboard)
				LAPP.ResetKeyboardInput();
			if (ImGui::GetIO().WantCaptureMouse)
				LAPP.ResetMouseInput();
		}
	}
	void drawEngine() {
		if (g_imgui_initialized && g_imgui_impl_dx11_initialized) {
			LAPP.getRenderer2D()->endBatch();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			LAPP.getRenderer2D()->beginBatch(); // restore
		}
	}

	void showTestInputWindow(bool* p_open)
	{
		auto* __p = LAPP.GetDInput();
		if (__p == nullptr) return;
		auto& dinput = *__p;

		static bool _first_set_size = false;
		if (!_first_set_size)
		{
			ImGui::SetNextWindowContentSize(ImVec2(640.0f, 480.0f));
			_first_set_size = true;
		}
		bool show = ImGui::Begin("InputTest##80FF", p_open);
		if (show)
		{
			static char buffer[1024] = { 0 };
			static wchar_t wbuffer[1024] = { 0 };
			static std::vector<std::string> combo_str;
			static std::vector<char*> combo_data;
			static int current_didx = 0;
			static int current_xidx = 0;
			static bool force_update = false;

			XINPUT_STATE xstate[4];
			ZeroMemory(xstate, sizeof(xstate));
			bool bxstate[4] = { false, false, false, false };
			DWORD xdevice = 0;
			for (size_t i = 0; i < 4; i += 1)
			{
				auto hr = Platform::XInput::getState((DWORD)i, xstate);
				if (hr == ERROR_SUCCESS)
				{
					bxstate[i] = true;
					xdevice += 1;
				}
			}

			ImGui::Checkbox("Force Update", &force_update);
			if (force_update)
			{
				dinput.update();
			}

			if (ImGui::BeginTabBar("##8010"))
			{
				if (ImGui::BeginTabItem("DirectInput##8011"))
				{
					{
						const auto cnt = dinput.count();
						combo_str.resize(cnt);
						combo_data.resize(cnt);
						for (uint32_t i = 0; i < cnt; i += 1)
						{
							auto s1 = dinput.getDeviceName(i);
							auto s2 = dinput.getProductName(i);
							swprintf(wbuffer, 1023, L"%u. %s (%s)", i + 1, s1 ? s1 : L"<null>", s2 ? s2 : L"<null>");
							int need = WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, NULL, 0, NULL, NULL);
							combo_str[i].resize(need);
							WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, (LPSTR)combo_str[i].data(), need, NULL, NULL);
							combo_data[i] = (char*)combo_str[i].c_str();
						}

						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
						ImGui::Combo("Devices", &current_didx, combo_data.data(), (int)combo_data.size());
						ImGui::SameLine();
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
						if (ImGui::Button("Refresh"))
						{
							dinput.refresh();
						}

						Platform::DirectInput::RawState state;
						if (dinput.getRawState(current_didx, &state))
						{
							Platform::DirectInput::AxisRange range;
							dinput.getAxisRange(current_didx, &range);

							int cache = 0;

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.lX;
							ImGui::SliderInt("Axis X", &cache, range.XMin, range.XMax);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.lY;
							ImGui::SliderInt("Axis Y", &cache, range.YMin, range.YMax);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.lZ;
							ImGui::SliderInt("Axis Z", &cache, range.ZMin, range.ZMax);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.lRx;
							ImGui::SliderInt("Axis RX", &cache, range.RxMin, range.RxMax);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.lRy;
							ImGui::SliderInt("Axis RY", &cache, range.RyMin, range.RyMax);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.lRz;
							ImGui::SliderInt("Axis RZ", &cache, range.RzMin, range.RzMax);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.rglSlider[0];
							ImGui::SliderInt("Slider 1", &cache, range.Slider0Min, range.Slider0Max);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = state.rglSlider[1];
							ImGui::SliderInt("Slider 2", &cache, range.Slider1Min, range.Slider1Max);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = (state.rgdwPOV[0] <= 36000) ? state.rgdwPOV[0] : 0;
							ImGui::SliderInt("POV 1", &cache, 0, (state.rgdwPOV[0] <= 36000) ? 36000 : 0);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = (state.rgdwPOV[1] <= 36000) ? state.rgdwPOV[1] : 0;
							ImGui::SliderInt("POV 2", &cache, 0, (state.rgdwPOV[1] <= 36000) ? 36000 : 0);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = (state.rgdwPOV[2] <= 36000) ? state.rgdwPOV[2] : 0;
							ImGui::SliderInt("POV 3", &cache, 0, (state.rgdwPOV[2] <= 36000) ? 36000 : 0);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache = (state.rgdwPOV[3] <= 36000) ? state.rgdwPOV[3] : 0;
							ImGui::SliderInt("POV 4", &cache, 0, (state.rgdwPOV[3] <= 36000) ? 36000 : 0);

							bool bcache = false;

						#define SHOWKEY(I, B) \
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.125f);\
								bcache = (state.rgbButtons[I - 1] != 0);\
								if constexpr ((I) < 10) ImGui::Checkbox("Button 0" #I, &bcache);\
								else ImGui::Checkbox("Button " #I, &bcache);\
								if constexpr (B) ImGui::SameLine();

							SHOWKEY(1, 1); SHOWKEY(2, 1); SHOWKEY(3, 1); SHOWKEY(4, 1); SHOWKEY(5, 1); SHOWKEY(6, 1); SHOWKEY(7, 1); SHOWKEY(8, 0);
							SHOWKEY(9, 1); SHOWKEY(10, 1); SHOWKEY(11, 1); SHOWKEY(12, 1); SHOWKEY(13, 1); SHOWKEY(14, 1); SHOWKEY(15, 1); SHOWKEY(16, 0);
							SHOWKEY(17, 1); SHOWKEY(18, 1); SHOWKEY(19, 1); SHOWKEY(20, 1); SHOWKEY(21, 1); SHOWKEY(22, 1); SHOWKEY(23, 1); SHOWKEY(24, 0);
							SHOWKEY(25, 1); SHOWKEY(26, 1); SHOWKEY(27, 1); SHOWKEY(28, 1); SHOWKEY(29, 1); SHOWKEY(30, 1); SHOWKEY(31, 1); SHOWKEY(32, 0);

						#undef SHOWKEY
						}
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("DirectInput to XInput##8012"))
				{
					{
						const auto cnt = dinput.count();
						combo_str.resize(cnt);
						combo_data.resize(cnt);
						for (uint32_t i = 0; i < cnt; i += 1)
						{
							auto s1 = dinput.getDeviceName(i);
							auto s2 = dinput.getProductName(i);
							swprintf(wbuffer, 1023, L"%u. %s (%s)", i + 1, s1 ? s1 : L"<null>", s2 ? s2 : L"<null>");
							int need = WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, NULL, 0, NULL, NULL);
							combo_str[i].resize(need);
							WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, (LPSTR)combo_str[i].data(), need, NULL, NULL);
							combo_data[i] = (char*)combo_str[i].c_str();
						}

						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
						ImGui::Combo("Devices", &current_didx, combo_data.data(), (int)combo_data.size());
						ImGui::SameLine();
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
						if (ImGui::Button("Refresh"))
						{
							dinput.refresh();
						}

						Platform::DirectInput::State state;
						if (dinput.getState(current_didx, &state))
						{
							SHORT cache[2];
							SHORT minv = -32768, maxv = 32767;
							BYTE bcache = 0;
							BYTE bminv = 0, bmaxv = 255;

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache[0] = state.sThumbLX; cache[1] = state.sThumbLY;
							ImGui::SliderScalarN("Left Joystick (LJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache[0] = state.sThumbRX; cache[1] = state.sThumbRY;
							ImGui::SliderScalarN("Right Joystick (RJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							bcache = state.bLeftTrigger;
							ImGui::SliderScalar("Left Trigger（LT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							bcache = state.bRightTrigger;
							ImGui::SliderScalar("Right Trigger（RT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);

							bool _bstate = false;

						#define SHOWKEY(L, NAME, C) \
								_bstate = ((state.wButtons & C) != 0);\
								ImGui::Checkbox(#NAME, &_bstate);\
								if constexpr ((L) != 0) ImGui::SameLine();

							SHOWKEY(1, UP, XINPUT_GAMEPAD_DPAD_UP);
							SHOWKEY(1, DOWN, XINPUT_GAMEPAD_DPAD_DOWN);
							SHOWKEY(1, LEFT, XINPUT_GAMEPAD_DPAD_LEFT);
							SHOWKEY(0, RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT);

							SHOWKEY(1, START, XINPUT_GAMEPAD_START);
							SHOWKEY(0, BACK, XINPUT_GAMEPAD_BACK);

							SHOWKEY(1, Left Thumb(LJB), XINPUT_GAMEPAD_LEFT_THUMB);
							SHOWKEY(0, Right Thumb(RJB), XINPUT_GAMEPAD_RIGHT_THUMB);

							SHOWKEY(1, Left Shoulder(LB), XINPUT_GAMEPAD_LEFT_SHOULDER);
							SHOWKEY(0, Right Shoulder(RB), XINPUT_GAMEPAD_RIGHT_SHOULDER);

							SHOWKEY(1, A, XINPUT_GAMEPAD_A);
							SHOWKEY(1, B, XINPUT_GAMEPAD_B);
							SHOWKEY(1, X, XINPUT_GAMEPAD_X);
							SHOWKEY(0, Y, XINPUT_GAMEPAD_Y);

						#undef SHOWKEY
						}
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("XInput##8013"))
				{
					{
						combo_str.resize(xdevice);
						combo_data.resize(xdevice);
						for (uint32_t i = 0; i < xdevice; i += 1)
						{
							snprintf(buffer, 1023, "%u. %s", i + 1, "XBox Controller & XInput Compatible Controller");
							combo_str[i] = buffer;
							combo_data[i] = (char*)combo_str[i].c_str();
						}

						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
						ImGui::Combo("Devices", &current_xidx, combo_data.data(), (int)combo_data.size());

						if (current_xidx < (int)xdevice)
						{
							auto& state = xstate[current_xidx].Gamepad;

							SHORT cache[2];
							SHORT minv = -32768, maxv = 32767;
							BYTE bcache = 0;
							BYTE bminv = 0, bmaxv = 255;

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache[0] = state.sThumbLX; cache[1] = state.sThumbLY;
							ImGui::SliderScalarN("Left Joystick (LJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							cache[0] = state.sThumbRX; cache[1] = state.sThumbRY;
							ImGui::SliderScalarN("Right Joystick (RJ)", ImGuiDataType_S16, cache, 2, &minv, &maxv);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							bcache = state.bLeftTrigger;
							ImGui::SliderScalar("Left Trigger（LT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
							bcache = state.bRightTrigger;
							ImGui::SliderScalar("Right Trigger（RT）", ImGuiDataType_U8, &bcache, &bminv, &bmaxv);

							bool _bstate = false;

						#define SHOWKEY(L, NAME, C) \
								_bstate = ((state.wButtons & C) != 0);\
								ImGui::Checkbox(#NAME, &_bstate);\
								if constexpr ((L) != 0) ImGui::SameLine();

							SHOWKEY(1, UP, XINPUT_GAMEPAD_DPAD_UP);
							SHOWKEY(1, DOWN, XINPUT_GAMEPAD_DPAD_DOWN);
							SHOWKEY(1, LEFT, XINPUT_GAMEPAD_DPAD_LEFT);
							SHOWKEY(0, RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT);

							SHOWKEY(1, START, XINPUT_GAMEPAD_START);
							SHOWKEY(0, BACK, XINPUT_GAMEPAD_BACK);

							SHOWKEY(1, Left Thumb(LJB), XINPUT_GAMEPAD_LEFT_THUMB);
							SHOWKEY(0, Right Thumb(RJB), XINPUT_GAMEPAD_RIGHT_THUMB);

							SHOWKEY(1, Left Shoulder(LB), XINPUT_GAMEPAD_LEFT_SHOULDER);
							SHOWKEY(0, Right Shoulder(RB), XINPUT_GAMEPAD_RIGHT_SHOULDER);

							SHOWKEY(1, A, XINPUT_GAMEPAD_A);
							SHOWKEY(1, B, XINPUT_GAMEPAD_B);
							SHOWKEY(1, X, XINPUT_GAMEPAD_X);
							SHOWKEY(0, Y, XINPUT_GAMEPAD_Y);

						#undef SHOWKEY
						}
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Keyboard##8014"))
				{
					for (int i = 0; i < 256; i += 1)
					{
						if (dinput.getKeyboardKeyState(i))
						{
							ImGui::Text("[%d]", i);
						}
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Mouse##8014"))
				{
					const auto x = dinput.getMouseMoveDeltaX();
					const auto y = dinput.getMouseMoveDeltaY();
					int xy[2] = { x, y };
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
					ImGui::DragInt2("Move Delta", xy);

					static int g_x = 0;
					static int g_y = 0;
					g_x += x;
					g_y += y;
					int g_xy[2] = { g_x, g_y };
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
					ImGui::DragInt2("Total Move", g_xy);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
					if (ImGui::Button("Reset##1"))
					{
						g_x = 0;
						g_y = 0;
					}

					int z = dinput.getMouseWheelDelta();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
					ImGui::DragInt("Wheel Delta", &z);

					static int g_z = 0;
					g_z += z;
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.8f);
					int _g_z = g_z;
					ImGui::DragInt("Total Wheel", &_g_z);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
					if (ImGui::Button("Reset##2"))
					{
						g_z = 0;
					}

					for (int i = 0; i < 8; i += 1)
					{
						if (dinput.getMouseKeyState(i))
						{
							ImGui::Text("[%d]", i);
						}
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}
};
