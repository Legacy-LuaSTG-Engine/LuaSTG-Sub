#pragma once
#include "Graphic/Device.h"

namespace slow::Graphic
{
    class DearImGuiRenderer
    {
    private:
        struct Implement;
        Implement* implememt = nullptr;
    public:
        bool createPipeline();
        void destroyPipeline();
        bool uploadFontAtlas();
        void update();
        bool uploadDrawData();
        void setRenderState();
        void draw();
    public:
        bool bind(Device& device);
        void unbind();
    public:
        DearImGuiRenderer();
        ~DearImGuiRenderer();
        static DearImGuiRenderer& get();
    };
};
