#pragma once
#include "GameResource/ResourceBase.hpp"
#include "Core/Graphics/Renderer.hpp"

namespace LuaSTGPlus
{
	struct IResourceModel : public IResourceBase
	{
		virtual Core::Graphics::IModel* GetModel() = 0;
	};
	
	class Mesh
	{
	private:
		std::vector<Core::Graphics::IRenderer::DrawVertex> vertex_;
		std::vector<Core::Graphics::IRenderer::DrawIndex> index_;
	public:
		Core::Graphics::IRenderer::DrawVertex* getVertexPointer() noexcept { return vertex_.data(); }
		Core::Graphics::IRenderer::DrawIndex* getIndexPointer() noexcept { return index_.data(); }
	public:
		bool resize(uint32_t vertex_count, uint32_t index_count) noexcept;
		uint32_t getVertexCount() const noexcept;
		uint32_t getIndexCount() const noexcept;
		void setAllVertexColor(Core::Color4B color) noexcept;
		void setIndex(uint32_t index, Core::Graphics::IRenderer::DrawIndex value) noexcept;
		void setVertex(uint32_t index, float x, float y, float z, float u, float v, Core::Color4B color) noexcept;
		void setVertexPosition(uint32_t index, float x, float y, float z) noexcept;
		void setVertexCoords(uint32_t index, float u, float v) noexcept;
		void setVertexColor(uint32_t index, Core::Color4B color) noexcept;
	public:
		bool draw(Core::Graphics::IRenderer* p_renderer);
		bool draw(Core::Graphics::IRenderer* p_renderer, Core::Graphics::ITexture2D* p_texture);
	public:
		Mesh();
		~Mesh();
	};
}
