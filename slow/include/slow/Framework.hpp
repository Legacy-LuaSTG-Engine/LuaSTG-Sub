#pragma once
#include "slow/Ref.hpp"
#include "slow/FileManager.hpp"
#include "slow/Window.hpp"

namespace slow {
    struct FrameworkCallback {
        virtual bool onCreate() = 0;
        virtual void onDestroy() = 0;
        virtual bool onUpdate() = 0;
        virtual void onDraw() = 0;
    };
    
    struct Framework : public Ref {
        virtual void setCallback(FrameworkCallback* callback) = 0;
        
        virtual FileManager* getFileManager() = 0;
        virtual Window* getWindow() = 0;
        
        virtual bool create() = 0;
        virtual void run() = 0;
        virtual void destroy() = 0;
        
        virtual void exit() = 0;
    };
};
