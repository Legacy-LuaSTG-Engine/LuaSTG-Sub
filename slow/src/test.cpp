#include "slow/ByteArray.hpp"
#include "slow/Graphics/Device.hpp"
#include "pch.hpp"

struct float4x4_view
{
    float data[16];
};

void test_1()
{
    slow::object_ptr<slow::IByteArray> blob;
    bool r = slow::createByteArray(~blob, 16 * sizeof(float), 256, false);
    float4x4_view* p = (float4x4_view*)blob->data();

    r = blob->writef32(1.0f, 0);
    r = blob->writef32(2.0f, 4);
    r = blob->writef32(3.0f, 8);
    r = blob->writef32(4.0f, 12);

    r = blob->writef32(5.0f, 16);
    r = blob->writef32(6.0f, 20);
    r = blob->writef32(7.0f, 24);
    r = blob->writef32(8.0f, 28);

    r = blob->writef32(0.1f, 32);
    r = blob->writef32(0.2f, 36);
    r = blob->writef32(0.3f, 40);
    r = blob->writef32(0.4f, 44);

    r = blob->writef32(0.5f, 48);
    r = blob->writef32(0.6f, 52);
    r = blob->writef32(0.7f, 56);
    r = blob->writef32(0.8f, 60);

    r = blob->writef32(0.9f, 5);

    float4x4_view va = {};

    r = blob->readf32(va.data + 0, 0);
    r = blob->readf32(va.data + 1, 4);
    r = blob->readf32(va.data + 2, 8);
    r = blob->readf32(va.data + 3, 12);

    r = blob->readf32(va.data + 4, 16);
    r = blob->readf32(va.data + 5, 20);
    r = blob->readf32(va.data + 6, 24);
    r = blob->readf32(va.data + 7, 28);

    r = blob->readf32(va.data + 8, 32);
    r = blob->readf32(va.data + 9, 36);
    r = blob->readf32(va.data + 10, 40);
    r = blob->readf32(va.data + 11, 44);

    r = blob->readf32(va.data + 12, 48);
    r = blob->readf32(va.data + 13, 52);
    r = blob->readf32(va.data + 14, 56);
    r = blob->readf32(va.data + 15, 60);

    r = blob->readf32(va.data + 0, 5);
}

int main(int, char**)
{
    using namespace slow;
    using namespace slow::Graphics;
    
    object_ptr<IDevice> dev;
    object_ptr<IPipelineState> state;
    if (createDevice(~dev))
    {
        
        
        TInputElement elements[2] = {
            TInputElement { "POSITION", 0, EFormat::R32G32_FLOAT, 0, 0, 0, EInputSlotType::PerVertexData },
            TInputElement { "TEXCOORD", 0, EFormat::R32G32_FLOAT, 0, 8, 0, EInputSlotType::PerVertexData },
        };
        TPipelineState def = {
            u8view(),
            u8view(),
            TInputLayout { elements, 2 },
            TRasterizerState {
                EFillMode::Solid,
                ECullMode::None,
                false,
                true,
                true,
                false,
                false,
                padding,
                0,
                0.0f,
                0.0f,
            },
            TDepthStencilState {
                false,
                EDepthWriteMask::All,
                EComparisonFunction::LessEqual,
                padding,
                false,
                0,
                0,
                padding,
                TDepthStencilOperation { EStencilOperation::Keep, EStencilOperation::Keep, EStencilOperation::Keep, EComparisonFunction::Always },
                TDepthStencilOperation { EStencilOperation::Keep, EStencilOperation::Keep, EStencilOperation::Keep, EComparisonFunction::Always },
            },
            TBlendState {
                true,
                EBlendFactor::One,
                EBlendFactor::PixelAlphaInv,
                EBlendOperation::Add,
                EBlendFactor::One,
                EBlendFactor::PixelAlphaInv,
                EBlendOperation::Add,
                EColorWriteMask::All,
            },
            0xFFFFFFFF,
        };
        
        dev->createPipelineState(def, ~state);
    }
    
    return 0;
}
