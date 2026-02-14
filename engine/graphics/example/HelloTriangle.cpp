#include "core/Application.hpp"
#include "core/FrameRateController.hpp"
#include "core/Logger.hpp"
#include "core/SmartReference.hpp"
#include "core/Window.hpp"
#include "core/GraphicsDevice.hpp"
#include "core/SwapChain.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "win32/base.hpp"
#include <cmath>
#include <numbers>
#include <DirectXMath.h>
using BYTE = uint8_t;
#include "shader/vertex_shader.h"
#include "shader/pixel_shader.h"

namespace {
	using std::string_view_literals::operator ""sv;

	class HelloTriangleApplication : public core::IApplication {
	public:
		struct Vertex {
			float x;
			float y;
			uint32_t color;
		};

		bool onCreate() override {
			core::Logger::info("[app] create"sv);

			if (!core::IWindow::create(m_window.put())) {
				return false;
			}
			if (!core::IGraphicsDevice::create(""sv, m_graphics_device.put())) {
				return false;
			}
			if (!core::ISwapChain::create(m_window.get(), m_graphics_device.get(), m_swap_chain.put())) {
				return false;
			}

			m_window->setWindowMode(core::Vector2U(640, 480));
			m_swap_chain->setWindowMode(core::Vector2U(640, 480));

			if (!createResources()) {
				return false;
			}

			return true;
		}
		void onBeforeUpdate() override {
			m_frame_rate_controller->update();
			m_swap_chain->waitFrameLatency();
		}
		bool onUpdate() override {
			constexpr float R = 100.0f;
			const float t = m_timer * 0.01f;
			Vertex vertex_buffer[4]{};
			vertex_buffer[0].x = R * std::cos(t);
			vertex_buffer[0].y = R * std::sin(t);
			vertex_buffer[1].x = R * std::cos(t - 0.5f * std::numbers::pi_v<float>);
			vertex_buffer[1].y = R * std::sin(t - 0.5f * std::numbers::pi_v<float>);
			vertex_buffer[2].x = R * std::cos(t - 1.0f * std::numbers::pi_v<float>);
			vertex_buffer[2].y = R * std::sin(t - 1.0f * std::numbers::pi_v<float>);
			vertex_buffer[3].x = R * std::cos(t - 1.5f * std::numbers::pi_v<float>);
			vertex_buffer[3].y = R * std::sin(t - 1.5f * std::numbers::pi_v<float>);
			vertex_buffer[0].color = 0xffff0000u;
			vertex_buffer[1].color = 0xff00ff00u;
			vertex_buffer[2].color = 0xff0000ffu;
			vertex_buffer[3].color = 0xffffffffu;
			if (void* ptr{}; m_vertex_buffer->map(&ptr, true)) {
				std::memcpy(ptr, vertex_buffer, sizeof(vertex_buffer));
				m_vertex_buffer->unmap();
			}
			else {
				return false;
			}

			uint16_t index_buffer[6]{};
			index_buffer[0] = 0;
			index_buffer[1] = 1;
			index_buffer[2] = 2;
			index_buffer[3] = 0;
			index_buffer[4] = 2;
			index_buffer[5] = 3;
			if (void* ptr{}; m_index_buffer->map(&ptr, true)) {
				std::memcpy(ptr, index_buffer, sizeof(index_buffer));
				m_index_buffer->unmap();
			}
			else {
				return false;
			}

			DirectX::XMFLOAT4X4A mvp{};
			DirectX::XMStoreFloat4x4A(&mvp, DirectX::XMMatrixOrthographicOffCenterLH(-320.0f, 320.0f, -240.0f, 240.0f, 0.0f, 1.0f));
			if (!m_mvp_matrix_buffer->update(&mvp, sizeof(mvp), true)) {
				return false;
			}

			m_swap_chain->applyRenderAttachment();
			m_swap_chain->clearRenderAttachment();

			const auto cmd = m_graphics_device->getCommandbuffer();

			cmd->bindVertexBuffer(0, m_vertex_buffer.get());
			cmd->bindIndexBuffer(m_index_buffer.get());

			cmd->bindVertexShaderConstantBuffer(0, m_mvp_matrix_buffer.get());

			cmd->setViewport(0.0f, 0.0f, 640.0f, 480.0f);
			cmd->setScissorRect(0, 0, 640u, 480u);

			cmd->bindGraphicsPipeline(m_graphics_pipeline.get());

			cmd->drawIndexed(6, 0, 0);

			m_swap_chain->present();
			m_timer += 1.0f;
			return true;
		}
		void onDestroy() override {
			core::Logger::info("[app] destroy"sv);
		}

	private:
		bool createResources() {
			if (!m_graphics_device->createVertexBuffer(4u * sizeof(Vertex), sizeof(Vertex), m_vertex_buffer.put())) {
				return false;
			}
			if (!m_graphics_device->createIndexBuffer(6u * sizeof(uint16_t), core::GraphicsFormat::r16_uint, m_index_buffer.put())) {
				return false;
			}
			if (!m_graphics_device->createConstantBuffer(16u * sizeof(float), m_mvp_matrix_buffer.put())) {
				return false;
			}

			#pragma region m_graphics_pipeline

			core::GraphicsVertexInputBuffer buffer{};
			buffer.slot = 0;
			buffer.stride = sizeof(Vertex);
			buffer.input_rate = core::GraphicsVertexInputRate::vertex;
			buffer.instance_step_rate = 0;

			core::GraphicsVertexInputElement elements[2]{};

			elements[0].semantic_name = "POSITION";
			elements[0].semantic_index = 0;
			elements[0].format = core::GraphicsFormat::r32_g32_float;
			elements[0].buffer_slot = 0;
			elements[0].offset = offsetof(Vertex, x);

			elements[1].semantic_name = "COLOR";
			elements[1].semantic_index = 0;
			elements[1].format = core::GraphicsFormat::b8_g8_r8_a8_unorm;
			elements[1].buffer_slot = 0;
			elements[1].offset = offsetof(Vertex, color);

			core::GraphicsPipelineState graphics_pipeline{};

			graphics_pipeline.vertex_input_state.buffers = &buffer;
			graphics_pipeline.vertex_input_state.buffer_count = 1;
			graphics_pipeline.vertex_input_state.elements = elements;
			graphics_pipeline.vertex_input_state.element_count = 2;

			graphics_pipeline.primitive_type = core::GraphicsPrimitiveType::triangle_list;

			graphics_pipeline.vertex_shader.data = vertex_shader;
			graphics_pipeline.vertex_shader.size = sizeof(vertex_shader);

			graphics_pipeline.rasterizer_state.cull_mode = core::GraphicsCullMode::back;

			graphics_pipeline.pixel_shader.data = pixel_shader;
			graphics_pipeline.pixel_shader.size = sizeof(pixel_shader);

			if (!m_graphics_device->createGraphicsPipeline(&graphics_pipeline, m_graphics_pipeline.put())) {
				return false;
			}

			#pragma endregion

			return true;
		}

		core::IFrameRateController* m_frame_rate_controller{ core::IFrameRateController::getInstance() };
		core::SmartReference<core::IWindow> m_window;
		core::SmartReference<core::IGraphicsDevice> m_graphics_device;
		core::SmartReference<core::ISwapChain> m_swap_chain;

		core::SmartReference<core::IGraphicsBuffer> m_vertex_buffer;
		core::SmartReference<core::IGraphicsBuffer> m_index_buffer;
		core::SmartReference<core::IGraphicsBuffer> m_mvp_matrix_buffer;
		core::SmartReference<core::IGraphicsPipeline> m_graphics_pipeline;

		float m_timer{};
	};
}

int main() {
	const auto logger = spdlog::stdout_color_mt("main", spdlog::color_mode::always);
	spdlog::set_default_logger(logger);
	win32::set_logger_writer([](const std::string_view message) -> void {
		spdlog::error(message);
	});

	HelloTriangleApplication app;
	core::ApplicationManager::run(&app);

	return 0;
}
