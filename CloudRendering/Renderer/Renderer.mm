// Objective-C++ stuff
#include "Renderer.hpp"
#import <QuartzCore/QuartzCore.h>

MTL::Drawable* Renderer::retrieve_next_drawable() {
    id<CAMetalDrawable> next_drawable = [((__bridge CAMetalLayer*)metal_layer) nextDrawable];
    return (__bridge MTL::Drawable*)next_drawable;
}
