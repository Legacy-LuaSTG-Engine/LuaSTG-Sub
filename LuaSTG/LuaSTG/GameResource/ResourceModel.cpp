#include "GameResource/ResourceModel.hpp"

namespace LuaSTGPlus
{
    bool Mesh::resize(uint32_t const vertex_count, uint32_t const index_count) noexcept
    {
        if (vertex_count > 32768 || index_count > 32768)
            return false;
        try
        {
            vertex_.resize(vertex_count);
            index_.resize(index_count);
        }
        catch (std::bad_alloc const&)
        {
            spdlog::error("[luastg] Mesh::Resize 失败，内存分配失败");
            return false;
        }
        catch (...)
        {
            spdlog::error("[luastg] Mesh::Resize 失败，未知错误");
            return false;
        }
        return true;
    }
    uint32_t Mesh::getVertexCount() const noexcept { return (uint32_t)vertex_.size(); }
    uint32_t Mesh::getIndexCount() const noexcept { return (uint32_t)index_.size(); }
    void Mesh::setAllVertexColor(Core::Color4B const color) noexcept
    {
        uint32_t const c = color.color();
        for (auto& v : vertex_) v.color = c;
    }
    void Mesh::setIndex(uint32_t const index, Core::Graphics::IRenderer::DrawIndex const value) noexcept
    {
        index_[index] = value;
    }
    void Mesh::setVertex(uint32_t const index, float const x, float const y, float const z, float const u, float const v, Core::Color4B const color) noexcept
    {
        vertex_[index].x = x;
        vertex_[index].y = y;
        vertex_[index].z = z;
        vertex_[index].color = color.color();
        vertex_[index].u = u;
        vertex_[index].v = v;
    }
    void Mesh::setVertexPosition(uint32_t const index, float const x, float const y, float const z) noexcept
    {
        vertex_[index].x = x;
        vertex_[index].y = y;
        vertex_[index].z = z;
    }
    void Mesh::setVertexCoords(uint32_t const index, float const u, float const v) noexcept
    {
        vertex_[index].u = u;
        vertex_[index].v = v;
    }
    void Mesh::setVertexColor(uint32_t const index, Core::Color4B const color) noexcept
    {
        vertex_[index].color = color.color();
    }

    bool Mesh::draw(Core::Graphics::IRenderer* p_renderer)
    {
        return p_renderer->drawRaw(
                vertex_.data(), (uint16_t)vertex_.size(),
                index_.data(), (uint16_t)index_.size());
    }
    bool Mesh::draw(Core::Graphics::IRenderer* p_renderer, Core::Graphics::ITexture2D* p_texture)
    {
        float const u_scale = 1.0f / (float)p_texture->getSize().x;
        float const v_scale = 1.0f / (float)p_texture->getSize().y;
        Core::Graphics::IRenderer::DrawVertex* p_vert = nullptr;
        Core::Graphics::IRenderer::DrawIndex* p_idx = nullptr;
        uint16_t vert_offset = 0;
        if (!p_renderer->drawRequest((uint16_t)vertex_.size(), (uint16_t)index_.size(), &p_vert, &p_idx, &vert_offset))
            return false;
        for (size_t i = 0; i < vertex_.size(); i += 1)
        {
            p_vert[i] = Core::Graphics::IRenderer::DrawVertex(
                    vertex_[i].x, vertex_[i].y, vertex_[i].z, vertex_[i].u * u_scale, vertex_[i].v * v_scale, vertex_[i].color);
        }
        for (size_t i = 0; i < index_.size(); i += 1)
        {
            p_idx[i] = vert_offset + index_[i];
        }
        return true;
    }

    Mesh::Mesh() = default;
    Mesh::~Mesh() = default;
}
