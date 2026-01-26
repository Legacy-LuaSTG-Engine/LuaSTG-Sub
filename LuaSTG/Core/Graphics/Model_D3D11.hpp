#pragma once
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include "Core/Graphics/Renderer.hpp"
#include "Core/Graphics/Direct3D11/Device.hpp"
#include "tiny_gltf.h"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace core::Graphics
{
    class ModelSharedComponent_D3D11
        : public implement::ReferenceCounted<IReferenceCounted>
        , public IDeviceEventListener
    {
        friend class Model_D3D11;
    private:
        SmartReference<Direct3D11::Device> m_device;

        win32::com_ptr<ID3D11ShaderResourceView> default_image;
        win32::com_ptr<ID3D11SamplerState> default_sampler;

        win32::com_ptr<ID3D11InputLayout> input_layout;
        win32::com_ptr<ID3D11InputLayout> input_layout_vc;
        win32::com_ptr<ID3D11VertexShader> shader_vertex;
        win32::com_ptr<ID3D11VertexShader> shader_vertex_vc;
        win32::com_ptr<ID3D11PixelShader> shader_pixel[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_alpha[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_inv_alpha[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_nt[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_alpha_nt[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_inv_alpha_nt[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_vc[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_alpha_vc[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_inv_alpha_vc[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_nt_vc[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_alpha_nt_vc[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_inv_alpha_nt_vc[IDX(IRenderer::FogState::MAX_COUNT)];

        win32::com_ptr<ID3D11PixelShader> shader_pixel_sd[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_sd_nt[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_sd_vc[IDX(IRenderer::FogState::MAX_COUNT)];
        win32::com_ptr<ID3D11PixelShader> shader_pixel_sd_nt_vc[IDX(IRenderer::FogState::MAX_COUNT)];

        win32::com_ptr<ID3D11RasterizerState> state_rs_cull_none;
        win32::com_ptr<ID3D11RasterizerState> state_rs_cull_back;
        win32::com_ptr<ID3D11DepthStencilState> state_ds_disable;
        win32::com_ptr<ID3D11DepthStencilState> state_ds;
        win32::com_ptr<ID3D11DepthStencilState> state_ds_no_write;
        win32::com_ptr<ID3D11DepthStencilState> state_ds_dl;
        win32::com_ptr<ID3D11BlendState> state_blend;
        win32::com_ptr<ID3D11BlendState> state_blend_alpha;

        win32::com_ptr<ID3D11Buffer> cbo_mvp;
        win32::com_ptr<ID3D11Buffer> cbo_mlw;
        win32::com_ptr<ID3D11Buffer> cbo_caminfo;
        win32::com_ptr<ID3D11Buffer> cbo_alpha;
        win32::com_ptr<ID3D11Buffer> cbo_light;

    private:
        bool createImage();
        bool createSampler();
        bool createShader();
        bool createConstantBuffer();
        bool createState();

        bool createResources();
        void onDeviceCreate();
        void onDeviceDestroy();

    public:
        ModelSharedComponent_D3D11(Direct3D11::Device* p_device);
        ~ModelSharedComponent_D3D11();
    };

    class Model_D3D11
        : public implement::ReferenceCounted<IModel>
        , public IDeviceEventListener
    {
    private:
        SmartReference<Direct3D11::Device> m_device;
        SmartReference<ModelSharedComponent_D3D11> shared_;

        DirectX::XMMATRIX t_scale_;
        DirectX::XMMATRIX t_trans_;
        DirectX::XMMATRIX t_mbrot_;

        std::string gltf_path;

        struct ModelBlock
        {
            win32::com_ptr<ID3D11Buffer> vertex_buffer;
            win32::com_ptr<ID3D11Buffer> uv_buffer;
            win32::com_ptr<ID3D11Buffer> normal_buffer;
            win32::com_ptr<ID3D11Buffer> color_buffer;
            win32::com_ptr<ID3D11Buffer> index_buffer;
            win32::com_ptr<ID3D11SamplerState> sampler;
            win32::com_ptr<ID3D11ShaderResourceView> image;
            DirectX::XMFLOAT4X4 local_matrix;
            DirectX::XMFLOAT4X4 local_matrix_normal; // notice: pair with local_matrix
            DirectX::XMFLOAT4 base_color;
            BOOL double_side = FALSE;
            BOOL alpha_blend = FALSE;
            BOOL alpha_mask = FALSE;
            FLOAT alpha = 0.5f;
            UINT draw_count = 0;
            DXGI_FORMAT index_format = DXGI_FORMAT_R16_UINT;
            D3D11_PRIMITIVE_TOPOLOGY primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            ModelBlock()
            {
                DirectX::XMStoreFloat4x4(&local_matrix, DirectX::XMMatrixIdentity());
                DirectX::XMStoreFloat4x4(&local_matrix_normal, DirectX::XMMatrixIdentity());
                base_color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
            }
        };
        struct Sunshine
        {
            DirectX::XMFLOAT4 ambient;
            DirectX::XMFLOAT4 pos;
            DirectX::XMFLOAT4 dir;
            DirectX::XMFLOAT4 color;

            void setDir(float dir_deg, float upd_deg)
            {
                DirectX::XMFLOAT3 slfrom(1.0f, 0.0f, 0.0f);
                auto rotate_vec2 = [](float& x, float& y, float r)
                {
                    float sin_v = std::sinf(r);
                    float cos_v = std::cosf(r);
                    float xx = x * cos_v - y * sin_v;
                    float yy = x * sin_v + y * cos_v;
                    x = xx;
                    y = yy;
                };
                // up and down
                rotate_vec2(slfrom.x, slfrom.y, DirectX::XMConvertToRadians(upd_deg));
                // direction
                rotate_vec2(slfrom.x, slfrom.z, DirectX::XMConvertToRadians(dir_deg));
                // inverse
                dir.x = -slfrom.x;
                dir.y = -slfrom.y;
                dir.z = -slfrom.z;
                dir.w = 0.0f;
            }

            Sunshine()
            {
                ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // full ambient light
                pos = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
                dir = DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
                color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f); // no directional light
            }
        };

        std::vector<win32::com_ptr<ID3D11ShaderResourceView>> image;
        std::vector<win32::com_ptr<ID3D11SamplerState>> sampler;

        std::vector<ModelBlock> model_block;

        Sunshine sunshine;

        std::vector<DirectX::XMMATRIX> mTRS_stack;

        bool processNode(tinygltf::Model& model, tinygltf::Node& node);
        bool createImage(tinygltf::Model& model);
        bool createSampler(tinygltf::Model& model);
        bool createModelBlock(tinygltf::Model& model);

        bool createResources();
        void onDeviceCreate();
        void onDeviceDestroy();

    public:

        void setAmbient(Vector3F const& color, float brightness);
        void setDirectionalLight(Vector3F const& direction, Vector3F const& color, float brightness);
        void setScaling(Vector3F const& scale);
        void setPosition(Vector3F const& pos);
        void setRotationRollPitchYaw(float roll, float pitch, float yaw);
        void setRotationQuaternion(Vector4F const& quat);

        void draw(IRenderer::FogState fog);

    public:
        Model_D3D11(Direct3D11::Device* p_device, ModelSharedComponent_D3D11* p_model_shared, StringView path);
        ~Model_D3D11();
    };
}

#undef IDX
