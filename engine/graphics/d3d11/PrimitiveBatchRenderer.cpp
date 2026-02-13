#include "d3d11/PrimitiveBatchRenderer.hpp"

namespace core {
    PrimitiveBatchRenderer::PrimitiveBatchRenderer() = default;
    PrimitiveBatchRenderer::~PrimitiveBatchRenderer() = default;

    bool PrimitiveBatchRenderer::createResources(IGraphicsDevice* const device) {
        if (device == nullptr) {
            assert(false); return false;
        }

        if (!m_device->createVertexBuffer(m_vertex_count * sizeof(DrawVertex), sizeof(DrawVertex), m_vertex_buffer.put())) {
            return false;
        }
        if (!m_device->createIndexBuffer(m_index_count * sizeof(DrawIndex), GraphicsFormat::r16_uint, m_index_buffer.put())) {
            return false;
        }

        m_initialized = true;
        return true;
    }

    bool PrimitiveBatchRenderer::beginBatch(const bool auto_draw) {
        if (m_batch_scope) {
            assert(false); return false;
        }
        const auto cycle = m_cycle_on_next_batch
            || m_vertex_current >= m_vertex_count
            || m_index_current >= m_index_count;
        if (!mapBuffers(cycle)) {
            return false;
        }
        if (m_auto_draw) {
            const auto cmd = m_device->getCommandbuffer();
            cmd->bindVertexBuffer(0, m_vertex_buffer.get());
            cmd->bindIndexBuffer(m_index_buffer.get());
        }
        m_batch_scope = true;
        m_auto_draw = auto_draw;
        m_cycle_on_next_batch = false;
        return true;
    }
    bool PrimitiveBatchRenderer::endBatch() {
        if (!m_batch_scope) {
            assert(false); return false;
        }
        m_batch_scope = false;
        if (!unmapBuffers()) {
            return false;
        }
        if (m_auto_draw) {
            drawOnly();
        }
        m_auto_draw = false;
        return true;
    }
    bool PrimitiveBatchRenderer::addTriangle(const DrawVertex& v1, const DrawVertex& v2, const DrawVertex& v3) {
        const DrawVertex vertices[3]{ v1, v2, v3 };
        return addTriangle(vertices);
    }
    bool PrimitiveBatchRenderer::addTriangle(const DrawVertex vertices[3]) {
        if (!m_batch_scope) {
            assert(false); return false;
        }
        if (m_vertex_current + 3 >= m_vertex_count || m_index_current + 3 >= m_index_count) {
            if (m_auto_draw) {
                if (!flush()) {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        const DrawIndex indices[3]{
            static_cast<DrawIndex>(m_vertex_current),
            static_cast<DrawIndex>(m_vertex_current) + 1,
            static_cast<DrawIndex>(m_vertex_current) + 2,
        };
        std::memcpy(m_vertex_pointer + m_vertex_current, vertices, sizeof(vertices));
        m_vertex_current += 3;
        std::memcpy(m_index_pointer + m_index_current, indices, sizeof(indices));
        m_index_current += 3;
        return true;
    }
    bool PrimitiveBatchRenderer::addQuad(const DrawVertex& v1, const DrawVertex& v2, const DrawVertex& v3, const DrawVertex& v4) {
        const DrawVertex vertices[4]{ v1, v2, v3, v4 };
        return addQuad(vertices);
    }
    bool PrimitiveBatchRenderer::addQuad(const DrawVertex vertices[4]) {
        if (!m_batch_scope) {
            assert(false); return false;
        }
        if (m_vertex_current + 4 >= m_vertex_count || m_index_current + 6 >= m_index_count) {
            if (m_auto_draw) {
                if (!flush()) {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        const DrawIndex indices[6]{
            static_cast<DrawIndex>(m_vertex_current),
            static_cast<DrawIndex>(m_vertex_current) + 1,
            static_cast<DrawIndex>(m_vertex_current) + 2,
            static_cast<DrawIndex>(m_vertex_current) + 2,
            static_cast<DrawIndex>(m_vertex_current) + 3,
            static_cast<DrawIndex>(m_vertex_current),
        };
        std::memcpy(m_vertex_pointer + m_vertex_current, vertices, sizeof(vertices));
        m_vertex_current += 4;
        std::memcpy(m_index_pointer + m_index_current, indices, sizeof(indices));
        m_index_current += 6;
        return true;
    }
    bool PrimitiveBatchRenderer::addRaw(const DrawVertex* const vertices, const size_t vertex_count, const DrawIndex* const indices, const size_t index_count) {
        if (!m_batch_scope) {
            assert(false); return false;
        }
        if (vertex_count > m_vertex_count || index_count > m_index_count) {
            assert(false); return false;
        }
        if (vertices == nullptr || indices == nullptr) {
            assert(false); return false;
        }
        if (m_vertex_current + vertex_count >= m_vertex_count || m_index_current + index_count >= m_index_count) {
            if (m_auto_draw) {
                if (!flush()) {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        const DrawIndex offset = static_cast<DrawIndex>(m_vertex_current);
        if (index_count > 0 && index_count <= 256) {
            DrawIndex modified_indices[256]; // uninitialize, ok
            for (size_t i = 0; i < index_count; i += 1) {
                modified_indices[i] = indices[i] + offset;
            }
            std::memcpy(m_index_pointer + m_index_current, modified_indices, sizeof(DrawIndex) * index_count);
            m_index_current += index_count;
        }
        else if (index_count > 256) {
            std::vector<DrawIndex> modified_indices(index_count);
            for (size_t i = 0; i < index_count; i += 1) {
                modified_indices[i] = indices[i] + offset;
            }
            std::memcpy(m_index_pointer + m_index_current, modified_indices.data(), sizeof(DrawIndex) * index_count);
            m_index_current += index_count;
        }
        if (vertex_count > 0) {
            std::memcpy(m_vertex_pointer + m_vertex_current, vertices, sizeof(DrawVertex) * vertex_count);
            m_vertex_current += vertex_count;
        }
        return true;
    }
    bool PrimitiveBatchRenderer::addRequest(const size_t vertex_count, const size_t index_count, DrawVertex** const out_vertices, DrawIndex** const out_indices, size_t* const out_index_offset) {
        if (!m_batch_scope) {
            assert(false); return false;
        }
        if (vertex_count > m_vertex_count || index_count > m_index_count) {
            assert(false); return false;
        }
        if (out_vertices == nullptr || out_indices == nullptr || out_index_offset == nullptr) {
            assert(false); return false;
        }
        if (m_vertex_current + vertex_count >= m_vertex_count || m_index_current + index_count >= m_index_count) {
            if (m_auto_draw) {
                if (!flush()) {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        *out_index_offset = m_vertex_current;
        *out_vertices = m_vertex_pointer + m_vertex_current;
        m_vertex_current += vertex_count;
        *out_indices = m_index_pointer + m_index_current;
        m_index_current += index_count;
        return true;
    }
    bool PrimitiveBatchRenderer::draw() {
        if (m_batch_scope) {
            assert(false); return false;
        }
        const auto cmd = m_device->getCommandbuffer();
        cmd->bindVertexBuffer(0, m_vertex_buffer.get());
        cmd->bindIndexBuffer(m_index_buffer.get());
        drawOnly();
        return true;
    }
    void PrimitiveBatchRenderer::clear() {
        m_vertex_begin = 0;
        m_vertex_current = 0;
        m_index_begin = 0;
        m_index_current = 0;
    }
    void PrimitiveBatchRenderer::setCycleOnNextBatch() {
        m_cycle_on_next_batch = true;
    }

    bool PrimitiveBatchRenderer::mapBuffers(const bool cycle) {
        if (!m_vertex_buffer->map(reinterpret_cast<void**>(&m_vertex_pointer), cycle)) {
            assert(false); return false;
        }
        if (!m_index_buffer->map(reinterpret_cast<void**>(&m_index_pointer), cycle)) {
            assert(false); return false;
        }
        return true;
    }
    bool PrimitiveBatchRenderer::unmapBuffers() {
        if (!m_vertex_buffer->unmap()) {
            assert(false); return false;
        }
        if (!m_index_buffer->unmap()) {
            assert(false); return false;
        }
        m_vertex_pointer = nullptr;
        m_index_pointer = nullptr;
        return true;
    }
    bool PrimitiveBatchRenderer::flush() {
        if (!unmapBuffers()) {
            return false;
        }
        drawOnly();
        clear();
        return mapBuffers(true);
    }
    void PrimitiveBatchRenderer::drawOnly() {
        const auto cmd = m_device->getCommandbuffer();
        if (const auto index_count = m_index_current - m_index_begin; index_count > 0) {
            cmd->drawIndexed(m_index_current - m_index_begin, m_index_begin, 0);
        }
        m_vertex_begin = m_vertex_current;
        m_index_begin = m_index_current;
    }
}
