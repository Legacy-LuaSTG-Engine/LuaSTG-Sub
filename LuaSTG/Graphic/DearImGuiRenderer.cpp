#include "Graphic/DearImGuiRenderer.h"
#include <cassert>
#include "imgui.h"
#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d11_4.h>
#include <DirectXMath.h>
#include "Graphic/DearImGuiRendererShader.inl"

namespace slow::Graphic {
    using Microsoft::WRL::ComPtr;
    
    struct DearImGuiRenderer::Implement {
        ComPtr<ID3D11Device> imDevice;
        ComPtr<ID3D11DeviceContext> imDeviceContext;
        
        ComPtr<ID3D11InputLayout> imInputLayout;
        ComPtr<ID3D11Buffer> imVertexBuffer;
        ComPtr<ID3D11Buffer> imIndexBuffer;
        size_t imVertexBufferSize = 4096u;
        size_t imIndexBufferSize = 8192u;
        
        ComPtr<ID3D11Buffer> imConstantBuffer;
        ComPtr<ID3D11VertexShader> imVertexShader;
        
        Pointer<IRasterizerState> imRasterizerState;
        
        Pointer<ISamplerState> imSamplerState;
        ComPtr<ID3D11ShaderResourceView> imFontAtlas;
        ComPtr<ID3D11PixelShader> imPixelShader;
        
        Pointer<IDepthStencilState> imDepthStencilState;
        Pointer<IBlendState> imBlendState;
        
        void reset() {
            imDevice.Reset();
            imDeviceContext.Reset();
            
            resetPipeline();
        };
        
        void resetPipeline() {
            imInputLayout.Reset();
            imVertexBuffer.Reset();
            imIndexBuffer.Reset();
            imVertexBufferSize = 4096u;
            imIndexBufferSize = 8192u;
            
            imConstantBuffer.Reset();
            imVertexShader.Reset();
            
            imRasterizerState.reset();
            
            imSamplerState.reset();
            imFontAtlas.Reset();
            imPixelShader.Reset();
            
            imDepthStencilState.reset();
            imBlendState.reset();
        };
    };
    
    #define self (*implememt)
    
    bool DearImGuiRenderer::createPipeline() {
        // check
        if (!self.imDevice) {
            return false;
        }
        ID3D11Device* dev_ = self.imDevice.Get();
        HRESULT hr;
        
        // input layout
        const D3D11_INPUT_ELEMENT_DESC layout_[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
            #ifdef IMGUI_USE_BGRA_PACKED_COLOR
            {"COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
            #else
            { "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            #endif
        };
        hr = dev_->CreateInputLayout(layout_, 3, g_VSBlob, sizeof(g_VSBlob),
                                     self.imInputLayout.ReleaseAndGetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        
        // shader
        hr = dev_->CreateVertexShader(g_VSBlob, sizeof(g_VSBlob), nullptr,
                                      self.imVertexShader.ReleaseAndGetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        hr = dev_->CreatePixelShader(g_PSBlob, sizeof(g_PSBlob), nullptr, self.imPixelShader.ReleaseAndGetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        
        // rasterizer
        DRasterizerState rs_;
        if (!Device::get().createRasterizerState(rs_, ~self.imRasterizerState)) {
            return false;
        }
        
        // sampler
        DSamplerState samp_;
        samp_.filter = ETextureFilter::Linear;
        samp_.addressU = samp_.addressV = samp_.addressW = ETextureAddress::Clamp;
        if (!Device::get().createSamplerState(samp_, ~self.imSamplerState)) {
            return false;
        }
        
        // depth
        DDepthStencilState ds_;
        if (!Device::get().createDepthStencilState(ds_, ~self.imDepthStencilState)) {
            return false;
        }
        
        // blend
        DBlendState blend_;
        blend_.enable = true;
        blend_.outputColor = EBlend::OutputAlpha;
        blend_.bufferColor = EBlend::InvOutputAlpha;
        blend_.outputAlpha = EBlend::One;
        blend_.bufferAlpha = EBlend::InvOutputAlpha;
        if (!Device::get().createBlendState(blend_, ~self.imBlendState)) {
            return false;
        }
        
        return true;
    }
    
    void DearImGuiRenderer::destroyPipeline() {
        self.resetPipeline();
    }
    
    bool DearImGuiRenderer::uploadFontAtlas() {
        // check
        if (!ImGui::GetCurrentContext()) {
            return false;
        }
        if (!self.imDevice) {
            return false;
        }
        ID3D11Device* dev_ = self.imDevice.Get();
        HRESULT hr;
        
        // pixel data
        ImGuiIO& io = ImGui::GetIO();
        uint8_t* pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        
        // upload
        D3D11_TEXTURE2D_DESC tex_;
        ZeroMemory(&tex_, sizeof(D3D11_TEXTURE2D_DESC));
        tex_.Width = width;
        tex_.Height = height;
        tex_.MipLevels = 1;
        tex_.ArraySize = 1;
        #ifdef IMGUI_USE_BGRA_PACKED_COLOR
        tex_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        #else
        tex_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        #endif
        tex_.SampleDesc.Count = 1;
        tex_.Usage = D3D11_USAGE_DEFAULT;
        tex_.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA res_;
        ZeroMemory(&res_, sizeof(D3D11_SUBRESOURCE_DATA));
        res_.pSysMem = pixels;
        res_.SysMemPitch = width * 4;
        ComPtr<ID3D11Texture2D> tex2d_;
        hr = dev_->CreateTexture2D(&tex_, &res_, tex2d_.GetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        
        // view
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_;
        ZeroMemory(&srv_, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        srv_.Format = tex_.Format;
        srv_.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_.Texture2D.MipLevels = tex_.MipLevels;
        hr = dev_->CreateShaderResourceView(tex2d_.Get(), &srv_, self.imFontAtlas.ReleaseAndGetAddressOf());
        if (hr != S_OK) {
            return false;
        }
        
        // setup back-end
        io.Fonts->SetTexID((ImTextureID) self.imFontAtlas.Get());
        
        return true;
    }
    
    void DearImGuiRenderer::update() {
        if (!self.imBlendState) {
            createPipeline();
        }
        if (!self.imFontAtlas) {
            uploadFontAtlas();
        }
    }
    
    bool DearImGuiRenderer::uploadDrawData() {
        // check
        if (!ImGui::GetCurrentContext()) {
            return false;
        }
        if (!self.imDevice || !self.imDeviceContext) {
            return false;
        }
        ImDrawData* draw_data_ = ImGui::GetDrawData();
        ID3D11Device* dev_ = self.imDevice.Get();
        ID3D11DeviceContext* ctx_ = self.imDeviceContext.Get();
        
        // create or grow VB
        if (!self.imVertexBuffer || self.imVertexBufferSize < draw_data_->TotalVtxCount) {
            while (self.imVertexBufferSize < draw_data_->TotalVtxCount) {
                self.imVertexBufferSize *= 2;
            }
            D3D11_BUFFER_DESC desc_;
            ZeroMemory(&desc_, sizeof(D3D11_BUFFER_DESC));
            desc_.ByteWidth = self.imVertexBufferSize * sizeof(ImDrawVert);
            desc_.Usage = D3D11_USAGE_DYNAMIC;
            desc_.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (S_OK != dev_->CreateBuffer(&desc_, nullptr, self.imVertexBuffer.ReleaseAndGetAddressOf())) {
                return false; // error
            }
        }
        // create or grow IB
        if (!self.imIndexBuffer || self.imIndexBufferSize < draw_data_->TotalIdxCount) {
            while (self.imIndexBufferSize < draw_data_->TotalIdxCount) {
                self.imIndexBufferSize *= 2;
            }
            D3D11_BUFFER_DESC desc_;
            ZeroMemory(&desc_, sizeof(D3D11_BUFFER_DESC));
            desc_.ByteWidth = self.imIndexBufferSize * sizeof(ImDrawIdx);
            desc_.Usage = D3D11_USAGE_DYNAMIC;
            desc_.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (S_OK != dev_->CreateBuffer(&desc_, nullptr, self.imIndexBuffer.ReleaseAndGetAddressOf())) {
                return false; // error
            }
        }
        
        // upload VB
        D3D11_MAPPED_SUBRESOURCE vb_ = {};
        if (S_OK == ctx_->Map(self.imVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vb_)) {
            auto* vtx_dst_ = (ImDrawVert*) vb_.pData;
            for (int n = 0; n < draw_data_->CmdListsCount; n++) {
                const ImDrawList* cmd_list_ = draw_data_->CmdLists[n];
                CopyMemory(vtx_dst_, cmd_list_->VtxBuffer.Data, cmd_list_->VtxBuffer.Size * sizeof(ImDrawVert));
                vtx_dst_ += cmd_list_->VtxBuffer.Size;
            }
            ctx_->Unmap(self.imVertexBuffer.Get(), 0);
        }
        else {
            return false; // error
        }
        // upload IB
        D3D11_MAPPED_SUBRESOURCE ib_ = {};
        if (S_OK == ctx_->Map(self.imIndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ib_)) {
            auto* idx_dst_ = (ImDrawIdx*) ib_.pData;
            for (int n = 0; n < draw_data_->CmdListsCount; n++) {
                const ImDrawList* cmd_list_ = draw_data_->CmdLists[n];
                CopyMemory(idx_dst_, cmd_list_->IdxBuffer.Data, cmd_list_->IdxBuffer.Size * sizeof(ImDrawIdx));
                idx_dst_ += cmd_list_->IdxBuffer.Size;
            }
            ctx_->Unmap(self.imIndexBuffer.Get(), 0);
        }
        else {
            return false; // error
        }
        
        // create CB
        if (!self.imConstantBuffer) {
            D3D11_BUFFER_DESC cbuf_;
            ZeroMemory(&cbuf_, sizeof(D3D11_BUFFER_DESC));
            cbuf_.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
            cbuf_.Usage = D3D11_USAGE_DYNAMIC;
            cbuf_.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbuf_.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (S_OK != dev_->CreateBuffer(&cbuf_, nullptr, self.imConstantBuffer.ReleaseAndGetAddressOf())) {
                return false; // error
            }
        }
        
        // upload constant buffer
        const DirectX::XMMATRIX proj_ = DirectX::XMMatrixOrthographicOffCenterLH(
            draw_data_->DisplayPos.x,
            draw_data_->DisplayPos.x + draw_data_->DisplaySize.x,
            draw_data_->DisplayPos.y + draw_data_->DisplaySize.y,
            draw_data_->DisplayPos.y,
            0.0f, 1.0f);
        DirectX::XMFLOAT4X4 mvp_ = {};
        DirectX::XMStoreFloat4x4(&mvp_, proj_);
        D3D11_MAPPED_SUBRESOURCE mvpdata_ = {};
        if (S_OK == ctx_->Map(self.imConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mvpdata_)) {
            CopyMemory(mvpdata_.pData, &mvp_, sizeof(DirectX::XMFLOAT4X4));
            ctx_->Unmap(self.imConstantBuffer.Get(), 0);
        }
        else {
            return false; // error
        }
        
        return true;
    }
    
    void DearImGuiRenderer::setRenderState() {
        // check
        if (!ImGui::GetCurrentContext()) {
            return;
        }
        if (!self.imDeviceContext) {
            return;
        }
        ImDrawData* draw_data_ = ImGui::GetDrawData();
        ID3D11DeviceContext* ctx_ = self.imDeviceContext.Get();
        
        // IA
        ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx_->IASetInputLayout(self.imInputLayout.Get());
        ID3D11Buffer* const buffer_[1] = {self.imVertexBuffer.Get()};
        UINT stride_[1] = {sizeof(ImDrawVert)};
        UINT offset_[1] = {0};
        ctx_->IASetVertexBuffers(0, 1, buffer_, stride_, offset_);
        assert(sizeof(ImDrawIdx) == 2 || sizeof(ImDrawIdx) == 4);
        DXGI_FORMAT format_ = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        ctx_->IASetIndexBuffer(self.imIndexBuffer.Get(), format_, 0);
        
        // VS
        ID3D11Buffer* const constant_[1] = {self.imConstantBuffer.Get()};
        ctx_->VSSetConstantBuffers(0, 1, constant_);
        ctx_->VSSetShader(self.imVertexShader.Get(), nullptr, 0);
        
        // RS
        const D3D11_VIEWPORT vp_[1] = {
            {
                0.0f, 0.0f,
                draw_data_->DisplaySize.x, draw_data_->DisplaySize.y,
                0.0f, 1.0f
            }
        };
        ctx_->RSSetViewports(1, vp_);
        Device::get().getContext().setRasterizerState(*self.imRasterizerState);
        
        // PS
        Device::get().getContext().setPixelShaderSampler(0, *self.imSamplerState);
        ctx_->PSSetShader(self.imPixelShader.Get(), nullptr, 0);
        
        // OM
        Device::get().getContext().setDepthStencilState(*self.imDepthStencilState);
        Device::get().getContext().setBlendState(*self.imBlendState);
    }
    
    void DearImGuiRenderer::draw() {
        // check
        if (!ImGui::GetCurrentContext()) {
            return;
        }
        if (!self.imFontAtlas) {
            return;
        }
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f) {
            return;
        }
        
        // upload data
        if (!uploadDrawData()) {
            return;
        }
        
        // render state
        setRenderState();
        
        // render
        int idx_offset_ = 0;
        int vtx_offset_ = 0;
        ImVec2 clip_pos_ = draw_data->DisplayPos;
        ID3D11DeviceContext* ctx_ = self.imDeviceContext.Get();
        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != nullptr) {
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                        setRenderState();
                    }
                    else {
                        pcmd->UserCallback(cmd_list, pcmd);
                    }
                }
                else {
                    const D3D11_RECT sr_[1] = {
                        {
                            (LONG) (pcmd->ClipRect.x - clip_pos_.x),
                            (LONG) (pcmd->ClipRect.y - clip_pos_.y),
                            (LONG) (pcmd->ClipRect.z - clip_pos_.x),
                            (LONG) (pcmd->ClipRect.w - clip_pos_.y),
                        }
                    };
                    ctx_->RSSetScissorRects(1, sr_);
                    ID3D11ShaderResourceView* const srv_[1] = {(ID3D11ShaderResourceView*) pcmd->TextureId};
                    ctx_->PSSetShaderResources(0, 1, srv_);
                    ctx_->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + idx_offset_, pcmd->VtxOffset + vtx_offset_);
                }
            }
            idx_offset_ += cmd_list->IdxBuffer.Size;
            vtx_offset_ += cmd_list->VtxBuffer.Size;
        }
    }
    
    bool DearImGuiRenderer::bind(Device& device) {
        unbind();
        
        // check
        if (!ImGui::GetCurrentContext()) {
            return false;
        }
        if (!device.validate()) {
            return false;
        }
        
        // setup back-end info
        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "slow.Graphic.DearImGuiRenderer (Direct3D11)";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        
        // get device
        
        self.imDevice = (ID3D11Device*) device.getHandle();
        self.imDevice->GetImmediateContext(self.imDeviceContext.ReleaseAndGetAddressOf());
        
        return true;
    }
    
    void DearImGuiRenderer::unbind() {
        self.reset();
        if (ImGui::GetCurrentContext()) {
            ImGuiIO& io = ImGui::GetIO();
            io.BackendRendererName = "<null>";
            if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset) {
                io.BackendFlags ^= ImGuiBackendFlags_RendererHasVtxOffset;
            }
            io.Fonts->SetTexID(nullptr);
        }
    }
    
    DearImGuiRenderer::DearImGuiRenderer() {
        implememt = new Implement;
    }
    
    DearImGuiRenderer::~DearImGuiRenderer() {
        unbind();
        delete implememt;
    }
    
    DearImGuiRenderer& DearImGuiRenderer::get() {
        static DearImGuiRenderer instance;
        return instance;
    }
}
