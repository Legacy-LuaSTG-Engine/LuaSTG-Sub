#include "d3d11/GraphicsBuffer.hpp"

namespace {
    using std::string_view_literals::operator ""sv;

    uint32_t alignTo16(const uint32_t size) {
        return ((size + 15u) / 16u) * 16;
    }
}

namespace core {
    // IGraphicsDeviceEventListener

    void VertexBuffer::onGraphicsDeviceCreate() {
        if (m_initialized) {
            createResources();
        }
    }
    void VertexBuffer::onGraphicsDeviceDestroy() {
        m_buffer.reset();
    }

    // IVertexBuffer

    void* VertexBuffer::getNativeResource() const {
        return m_buffer.get();
    }
    void* VertexBuffer::getNativeView() const {
        return nullptr;
    }

    uint32_t VertexBuffer::getSizeInBytes() const {
        return m_size_in_bytes;
    }
    uint32_t VertexBuffer::getStrideInBytes() const {
        return m_stride_in_bytes;
    }
    bool VertexBuffer::map(void** const out_pointer, const bool cycle) {
        if (!out_pointer) {
            assert(false);
            return false;
        }
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (!win32::check_hresult_as_boolean(
            context->Map(
                m_buffer.get(),
                0, // sub resource
                cycle ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE,
                0, // flags
                &mapped
            ),
            "ID3D11DeviceContext::Map"sv
        )) {
            return false;
        }
        *out_pointer = mapped.pData;
        return true;
    }
    bool VertexBuffer::unmap() {
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        context->Unmap(m_buffer.get(), 0);
        return true;
    }
    bool VertexBuffer::update(const void* data, uint32_t size, bool cycle) {
        if (size == 0) {
            return true;
        }
        if (size > m_size_in_bytes) {
            assert(false);
            return false;
        }
        if (!data) {
            assert(false);
            return false;
        }
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (!win32::check_hresult_as_boolean(
            context->Map(m_buffer.get(), 0, cycle ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped),
            "ID3D11DeviceContext::Map"sv
        )) {
            return false;
        }
        std::memcpy(mapped.pData, data, size);
        context->Unmap(m_buffer.get(), 0);
        return true;
    }

    // VertexBuffer

    VertexBuffer::VertexBuffer() = default;
    VertexBuffer::~VertexBuffer() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
    }

    bool VertexBuffer::createResources() {
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_BUFFER_DESC info{};
        info.ByteWidth = m_size_in_bytes;
        info.Usage = D3D11_USAGE_DYNAMIC;
        info.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        return win32::check_hresult_as_boolean(
            device->CreateBuffer(&info, nullptr, m_buffer.put()),
            "ID3D11Device::CreateBuffer"sv
        );
    }
    bool VertexBuffer::createResources(IGraphicsDevice* const device, const uint32_t size_in_bytes, const uint32_t stride_in_bytes) {
        if (device == nullptr) {
            assert(false); return false;
        }
        if (size_in_bytes == 0) {
            assert(false); return false;
        }
        if (stride_in_bytes == 0) {
            assert(false); return false;
        }
        m_device = device;
        m_size_in_bytes = size_in_bytes;
        m_stride_in_bytes = stride_in_bytes;
        if (!createResources()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
}

namespace core {
    // IGraphicsDeviceEventListener

    void IndexBuffer::onGraphicsDeviceCreate() {
        if (m_initialized) {
            createResources();
        }
    }
    void IndexBuffer::onGraphicsDeviceDestroy() {
        m_buffer.reset();
    }

    // IIndexBuffer

    void* IndexBuffer::getNativeResource() const {
        return m_buffer.get();
    }
    void* IndexBuffer::getNativeView() const {
        return nullptr;
    }

    uint32_t IndexBuffer::getSizeInBytes() const {
        return m_size_in_bytes;
    }
    uint32_t IndexBuffer::getStrideInBytes() const {
        return 0; // FIXME
    }
    bool IndexBuffer::map(void** const out_pointer, const bool cycle) {
        if (!out_pointer) {
            assert(false);
            return false;
        }
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (!win32::check_hresult_as_boolean(
            context->Map(
                m_buffer.get(),
                0, // sub resource
                cycle ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE,
                0, // flags
                &mapped
            ),
            "ID3D11DeviceContext::Map"sv
        )) {
            return false;
        }
        *out_pointer = mapped.pData;
        return true;
    }
    bool IndexBuffer::unmap() {
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        context->Unmap(m_buffer.get(), 0);
        return true;
    }
    bool IndexBuffer::update(const void* data, uint32_t size, bool cycle) {
        if (size == 0) {
            return true;
        }
        if (size > m_size_in_bytes) {
            assert(false);
            return false;
        }
        if (!data) {
            assert(false);
            return false;
        }
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (!win32::check_hresult_as_boolean(
            context->Map(m_buffer.get(), 0, cycle ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapped),
            "ID3D11DeviceContext::Map"sv
        )) {
            return false;
        }
        std::memcpy(mapped.pData, data, size);
        context->Unmap(m_buffer.get(), 0);
        return true;
    }

    // IndexBuffer

    IndexBuffer::IndexBuffer() = default;
    IndexBuffer::~IndexBuffer() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
    }

    bool IndexBuffer::createResources() {
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_BUFFER_DESC info{};
        info.ByteWidth = m_size_in_bytes;
        info.Usage = D3D11_USAGE_DYNAMIC;
        info.BindFlags = D3D11_BIND_INDEX_BUFFER;
        info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        return win32::check_hresult_as_boolean(
            device->CreateBuffer(&info, nullptr, m_buffer.put()),
            "ID3D11Device::CreateBuffer"sv
        );
    }
    bool IndexBuffer::createResources(IGraphicsDevice* const device, uint32_t const size_in_bytes) {
        if (!device) {
            assert(false);
            return false;
        }
        if (size_in_bytes == 0) {
            assert(false);
            return false;
        }
        m_device = device;
        m_size_in_bytes = size_in_bytes;
        if (!createResources()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
}

namespace core {
    // IGraphicsDeviceEventListener

    void ConstantBuffer::onGraphicsDeviceCreate() {
        if (m_initialized) {
            createResources();
        }
    }
    void ConstantBuffer::onGraphicsDeviceDestroy() {
        m_buffer.reset();
    }

    // IConstantBuffer

    void* ConstantBuffer::getNativeResource() const {
        return m_buffer.get();
    }
    void* ConstantBuffer::getNativeView() const {
        return nullptr;
    }

    uint32_t ConstantBuffer::getSizeInBytes() const {
        return m_size_in_bytes;
    }
    uint32_t ConstantBuffer::getStrideInBytes() const {
        return 0; // FIXME
    }
    bool ConstantBuffer::map(void** const out_pointer, const bool cycle) {
        if (!out_pointer) {
            assert(false);
            return false;
        }
        if (!cycle) {
            assert(false);
            return false;
        }
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (!win32::check_hresult_as_boolean(
            context->Map(
                m_buffer.get(),
                0, // sub resource
                cycle ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE,
                0, // flags
                &mapped
            ),
            "ID3D11DeviceContext::Map"sv
        )) {
            return false;
        }
        *out_pointer = mapped.pData;
        return true;
    }
    bool ConstantBuffer::unmap() {
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        context->Unmap(m_buffer.get(), 0);
        return true;
    }
    bool ConstantBuffer::update(const void* data, uint32_t size, bool cycle) {
        if (size == 0) {
            return true;
        }
        if (size > m_size_in_bytes) {
            assert(false);
            return false;
        }
        if (!data) {
            assert(false);
            return false;
        }
        if (!cycle) {
            assert(false);
            return false;
        }
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (!win32::check_hresult_as_boolean(
            context->Map(m_buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
            "ID3D11DeviceContext::Map"sv
        )) {
            return false;
        }
        std::memcpy(mapped.pData, data, size);
        context->Unmap(m_buffer.get(), 0);
        return true;
    }

    // ConstantBuffer

    ConstantBuffer::ConstantBuffer() = default;
    ConstantBuffer::~ConstantBuffer() {
        if (m_initialized && m_device) {
            m_device->removeEventListener(this);
        }
    }

    bool ConstantBuffer::createResources() {
        auto const device = static_cast<ID3D11Device*>(m_device->getNativeDevice());
        if (!device) {
            assert(false);
            return false;
        }
        win32::com_ptr<ID3D11DeviceContext> context;
        device->GetImmediateContext(context.put());
        if (!context) {
            assert(false);
            return false;
        }
        D3D11_BUFFER_DESC info{};
        // For a constant buffer (BindFlags of D3D11_BUFFER_DESC set to D3D11_BIND_CONSTANT_BUFFER),
        // you must set the ByteWidth value of D3D11_BUFFER_DESC in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.
        // https://learn.microsoft.com/en-us/windows/win32/api/D3D11/nf-d3d11-id3d11device-createbuffer
        info.ByteWidth = alignTo16(m_size_in_bytes);
        info.Usage = D3D11_USAGE_DYNAMIC;
        info.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        info.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        return win32::check_hresult_as_boolean(
            device->CreateBuffer(&info, nullptr, m_buffer.put()),
            "ID3D11Device::CreateBuffer"sv
        );
    }
    bool ConstantBuffer::createResources(IGraphicsDevice* const device, uint32_t const size_in_bytes) {
        if (!device) {
            assert(false);
            return false;
        }
        if (size_in_bytes == 0) {
            assert(false);
            return false;
        }
        m_device = device;
        m_size_in_bytes = size_in_bytes;
        if (!createResources()) {
            return false;
        }
        m_initialized = true;
        m_device->addEventListener(this);
        return true;
    }
}

#include "d3d11/GraphicsDevice.hpp"

namespace core {
    bool GraphicsDevice::createVertexBuffer(const uint32_t size_in_bytes, const uint32_t stride_in_bytes, IGraphicsBuffer** const output_buffer) {
        if (!output_buffer) {
            assert(false);
            return false;
        }
        *output_buffer = nullptr;
        SmartReference<VertexBuffer> buffer;
        buffer.attach(new VertexBuffer());
        if (!buffer->createResources(this, size_in_bytes, stride_in_bytes)) {
            return false;
        }
        *output_buffer = buffer.detach();
        return true;
    }
    bool GraphicsDevice::createIndexBuffer(const uint32_t size_in_bytes, IGraphicsBuffer** const output_buffer) {
        if (!output_buffer) {
            assert(false);
            return false;
        }
        *output_buffer = nullptr;
        SmartReference<IndexBuffer> buffer;
        buffer.attach(new IndexBuffer());
        if (!buffer->createResources(this, size_in_bytes)) {
            return false;
        }
        *output_buffer = buffer.detach();
        return true;
    }
    bool GraphicsDevice::createConstantBuffer(const uint32_t size_in_bytes, IGraphicsBuffer** const output_buffer) {
        if (!output_buffer) {
            assert(false);
            return false;
        }
        *output_buffer = nullptr;
        SmartReference<ConstantBuffer> buffer;
        buffer.attach(new ConstantBuffer());
        if (!buffer->createResources(this, size_in_bytes)) {
            return false;
        }
        *output_buffer = buffer.detach();
        return true;
    }
}
