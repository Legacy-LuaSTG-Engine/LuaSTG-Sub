#include "Debugger/FrameQuery.hpp"
#include "core/SmartReference.hpp"
#include "win32/base.hpp"
#include <cassert>
#include <d3d11.h>

namespace luastg {
    struct FrameQuery::Impl : public core::IGraphicsDeviceEventListener {
    public:
        // IGraphicsDeviceEventListener

        void onGraphicsDeviceCreate() override {
            createResources(static_cast<ID3D11Device*>(m_device->getNativeHandle()));
        }
        void onGraphicsDeviceDestroy() override {
            destroyResources();
        }

        // FrameQuery

        bool createResources(ID3D11Device* const device) {
            assert(device);
            d3d11_device = device;
            d3d11_device->GetImmediateContext(d3d11_device_context.put());
            D3D11_QUERY_DESC d3d11_query_info{};
            d3d11_query_info.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
            if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_freq.put())) {
                return false;
            }
            d3d11_query_info.Query = D3D11_QUERY_TIMESTAMP;
            if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_time_begin.put())) {
                return false;
            }
            if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_time_end.put())) {
                return false;
            }
            d3d11_query_info.Query = D3D11_QUERY_PIPELINE_STATISTICS;
            if (S_OK != d3d11_device->CreateQuery(&d3d11_query_info, d3d11_query_statistics.put())) {
                return false;
            }
            is_flying = false;
            is_scope = false;
            return true;
        }
        void destroyResources() {
            d3d11_query_freq.reset();
            d3d11_query_time_begin.reset();
            d3d11_query_time_end.reset();
            d3d11_query_statistics.reset();
            is_flying = false;
            is_scope = false;
        }
        void fetchData() {
            assert(d3d11_device_context);
            assert(d3d11_query_freq);
            assert(d3d11_query_time_begin);
            assert(d3d11_query_time_end);
            assert(d3d11_query_statistics);
            if (!is_flying) {
                return;
            }
            if (d3d11_query_freq) {
                while (S_OK != d3d11_device_context->GetData(d3d11_query_freq.get(), &freq, sizeof(freq), 0)) {
                    ;
                }
            }
            if (d3d11_query_time_begin) {
                while (S_OK != d3d11_device_context->GetData(d3d11_query_time_begin.get(), &time_begin, sizeof(time_begin), 0)) {
                    ;
                }
            }
            if (d3d11_query_time_end) {
                while (S_OK != d3d11_device_context->GetData(d3d11_query_time_end.get(), &time_end, sizeof(time_end), 0)) {
                    ;
                }
            }
            if (d3d11_query_statistics) {
                while (S_OK != d3d11_device_context->GetData(d3d11_query_statistics.get(), &statistics, sizeof(statistics), 0)) {
                    ;
                }
            }
            is_flying = false;
        }
        double getTime() {
            if (freq.Frequency == 0) {
                return 0.0;
            }
            return double(time_end - time_begin) / double(freq.Frequency);
        }
        void begin() {
            if (is_flying) {
                fetchData();
            }
            assert(!is_flying);
            assert(!is_scope);
            is_scope = true;
            assert(d3d11_device_context);
            assert(d3d11_query_freq);
            assert(d3d11_query_time_begin);
            assert(d3d11_query_statistics);
            if (d3d11_query_freq) {
                d3d11_device_context->Begin(d3d11_query_freq.get());
            }
            if (d3d11_query_time_begin) {
                d3d11_device_context->End(d3d11_query_time_begin.get()); // timestamp End only
            }
            if (d3d11_query_statistics) {
                d3d11_device_context->Begin(d3d11_query_statistics.get());
            }
        }
        void end() {
            assert(d3d11_device_context);
            assert(d3d11_query_freq);
            assert(d3d11_query_time_end);
            assert(d3d11_query_statistics);
            assert(is_scope);
            if (d3d11_query_freq) {
                d3d11_device_context->End(d3d11_query_freq.get());
            }
            if (d3d11_query_time_end) {
                d3d11_device_context->End(d3d11_query_time_end.get()); // timestamp End only
            }
            if (d3d11_query_statistics) {
                d3d11_device_context->End(d3d11_query_statistics.get());
            }
            is_scope = false;
            is_flying = true;
        }

        Impl(core::IGraphicsDevice* device) : m_device(device) {
            createResources(static_cast<ID3D11Device*>(m_device->getNativeHandle()));
            m_device->addEventListener(this);
        }
        ~Impl() {
            m_device->removeEventListener(this);
        }

    private:
        core::SmartReference<core::IGraphicsDevice> m_device;
        win32::com_ptr<ID3D11Device> d3d11_device;
        win32::com_ptr<ID3D11DeviceContext> d3d11_device_context;
        win32::com_ptr<ID3D11Query> d3d11_query_freq;
        win32::com_ptr<ID3D11Query> d3d11_query_time_begin;
        win32::com_ptr<ID3D11Query> d3d11_query_time_end;
        win32::com_ptr<ID3D11Query> d3d11_query_statistics;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT freq{};
        UINT64 time_begin{};
        UINT64 time_end{};
        D3D11_QUERY_DATA_PIPELINE_STATISTICS statistics{};
        bool is_flying{};
        bool is_scope{};
    };

    void FrameQuery::fetchData() { m_impl->fetchData(); }
    double FrameQuery::getTime() { return m_impl->getTime(); }
    void FrameQuery::begin()  { m_impl->begin(); }
    void FrameQuery::end()  { m_impl->end(); }

    FrameQuery::FrameQuery(core::IGraphicsDevice* device) {
        m_impl = new Impl(device);
    }
    FrameQuery::~FrameQuery() {
        delete m_impl;
    }
}
