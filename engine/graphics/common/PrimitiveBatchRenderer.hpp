#pragma once
#include "core/GraphicsDevice.hpp"
#include "core/Graphics/Renderer.hpp"
#include "core/SmartReference.hpp"

namespace core {
    class PrimitiveBatchRenderer : public IGraphicsDeviceEventListener {
    public:
        using DrawVertex = Graphics::IRenderer::DrawVertex;
        using DrawIndex = Graphics::IRenderer::DrawIndex;

        void onGraphicsDeviceCreate() override;
        void onGraphicsDeviceDestroy() override;

        PrimitiveBatchRenderer();
        PrimitiveBatchRenderer(const PrimitiveBatchRenderer&) = delete;
        PrimitiveBatchRenderer(PrimitiveBatchRenderer&&) = delete;
        ~PrimitiveBatchRenderer();

        PrimitiveBatchRenderer& operator=(const PrimitiveBatchRenderer&) = delete;
        PrimitiveBatchRenderer& operator=(PrimitiveBatchRenderer&&) = delete;

        // [free]
        bool createResources(IGraphicsDevice* device);

        // [free]
        bool isBatch() const noexcept;
        // [free]
        bool beginBatch(bool auto_draw);
        // [batch]
        bool endBatch();
        // [batch]
        bool consume();
        // [batch]
        bool addTriangle(const DrawVertex& v1, const DrawVertex& v2, const DrawVertex& v3);
        // [batch]
        bool addTriangle(const DrawVertex vertices[3]);
        // [batch]
        bool addQuad(const DrawVertex& v1, const DrawVertex& v2, const DrawVertex& v3, const DrawVertex& v4);
        // [batch]
        bool addQuad(const DrawVertex vertices[4]);
        // [batch]
        bool addRaw(const DrawVertex* vertices, size_t vertex_count, const DrawIndex* indices, size_t index_count);
        // [batch]
        bool addRequest(size_t vertex_count, size_t index_count, DrawVertex** out_vertices, DrawIndex** out_indices, size_t* out_index_offset);
        // [free]
        bool draw();
        // [free] [batch]
        void discard();
        // [free] [batch]
        void setCycleOnNextBatch();

    private:
        // [free] [batch]
        bool mapBuffers(bool cycle);
        // [free] [batch]
        bool unmapBuffers();
        // [batch]
        bool flush(bool cycle);
        // [free] [batch]
        void drawOnly();

        SmartReference<IGraphicsDevice> m_device;
        SmartReference<IGraphicsBuffer> m_vertex_buffer;
        SmartReference<IGraphicsBuffer> m_index_buffer;
        size_t m_vertex_count{ 60000 };
        size_t m_index_count{ 90000 };
        DrawVertex* m_vertex_pointer{};
        size_t m_vertex_begin{};
        size_t m_vertex_current{};
        DrawIndex* m_index_pointer{};
        size_t m_index_begin{};
        size_t m_index_current{};
        bool m_initialized{};
        bool m_batch_scope{};
        bool m_auto_draw{};
        bool m_cycle_on_next_batch{};
    };
}
