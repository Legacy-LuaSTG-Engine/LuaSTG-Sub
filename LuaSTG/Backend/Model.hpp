#pragma once
#include "Core/Renderer.hpp"
#include "tiny_gltf.h"

#define IDX(x) (size_t)static_cast<uint8_t>(x)

namespace LuaSTG::Core
{
    class ModelSharedComponent : public IObject
    {
        friend class Model;
    private:
        volatile intptr_t ref_;
    public:
        intptr_t retain();
        intptr_t release();
    private:
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> default_image;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> default_sampler;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout_vc;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> shader_vertex;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> shader_vertex_vc;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel[IDX(FogState::MAX_COUNT)];
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel_alpha[IDX(FogState::MAX_COUNT)];
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel_nt[IDX(FogState::MAX_COUNT)];
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel_alpha_nt[IDX(FogState::MAX_COUNT)];
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel_vc[IDX(FogState::MAX_COUNT)];
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel_alpha_vc[IDX(FogState::MAX_COUNT)];
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel_nt_vc[IDX(FogState::MAX_COUNT)];
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_pixel_alpha_nt_vc[IDX(FogState::MAX_COUNT)];

        Microsoft::WRL::ComPtr<ID3D11RasterizerState> state_rs_cull_none;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> state_rs_cull_back;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state_ds;
        Microsoft::WRL::ComPtr<ID3D11BlendState> state_blend;
        Microsoft::WRL::ComPtr<ID3D11BlendState> state_blend_alpha;

        Microsoft::WRL::ComPtr<ID3D11Buffer> cbo_mvp;
        Microsoft::WRL::ComPtr<ID3D11Buffer> cbo_mlw;
        Microsoft::WRL::ComPtr<ID3D11Buffer> cbo_caminfo;
        Microsoft::WRL::ComPtr<ID3D11Buffer> cbo_alpha;
        Microsoft::WRL::ComPtr<ID3D11Buffer> cbo_light;
    private:
        bool createImage();
        bool createSampler();
        bool createShader();
        bool createConstantBuffer();
        bool createState();
    public:
        bool attachDevice(ID3D11Device* dev);
        void detachDevice();
    public:
        ModelSharedComponent();
        ~ModelSharedComponent();
    };
    
	class Model : public IModel
	{
	private:
		volatile intptr_t ref_;
	public:
		intptr_t retain();
		intptr_t release();
	private:
		DirectX::XMMATRIX t_scale_;
		DirectX::XMMATRIX t_trans_;
		DirectX::XMMATRIX t_mbrot_;
	public:
        void setAmbient(Vector3 const& color, float brightness);
        void setDirectionalLight(Vector3 const& direction, Vector3 const& color, float brightness);
		void setScaling(Vector3 const& scale);
		void setPosition(Vector3 const& pos);
		void setRotationRollPitchYaw(float roll, float pitch, float yaw);
		void setRotationQuaternion(Vector4 const& quat);
	private:
        ScopeObject<ModelSharedComponent> shared_;

        std::string gltf_path;
        
        struct ModelBlock
        {
            Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
            Microsoft::WRL::ComPtr<ID3D11Buffer> uv_buffer;
            Microsoft::WRL::ComPtr<ID3D11Buffer> normal_buffer;
            Microsoft::WRL::ComPtr<ID3D11Buffer> color_buffer;
            Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
            Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> image;
            DirectX::XMFLOAT4X4 local_matrix;
            DirectX::XMFLOAT4X4 local_matrix_normal; // notice: pair with local_matrix
            DirectX::XMFLOAT4 base_color;
            BOOL double_side = FALSE;
            BOOL alpha_blend = FALSE;
            BOOL alpha_cull = FALSE;
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

        std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> image;
        std::vector<Microsoft::WRL::ComPtr<ID3D11SamplerState>> sampler;

        std::vector<ModelBlock> model_block;

        Sunshine sunshine;

        std::vector<DirectX::XMMATRIX> mTRS_stack;
        bool processNode(tinygltf::Model& model, tinygltf::Node& node);
        bool createImage(tinygltf::Model& model);
        bool createSampler(tinygltf::Model& model);
        bool createModelBlock(tinygltf::Model& model);
    public:
        bool attachDevice();
        void detachDevice();
	public:
		void draw(FogState fog);
	public:
		Model(std::string_view const& path, ScopeObject<ModelSharedComponent> model_shared);
		~Model();
	};
}

#undef IDX
