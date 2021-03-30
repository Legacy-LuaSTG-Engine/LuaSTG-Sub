#include "Graphic/Test.h"

namespace slow::Graphic {
    bool _bindEngine(void* window) {
        return true;
    }
    
    void _unbindEngine() {}
    
    bool _update() {
        return true;
    }
    
    bool _draw() {
        return false;
    }
}
