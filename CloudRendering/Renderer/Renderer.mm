// why aren't these in metal-cpp???
#include "Renderer.hpp"
#import <QuartzCore/QuartzCore.h>

CA::MetalDrawable* Renderer::retrieve_next_drawable() {
    __block id<CAMetalDrawable> next_drawable;
    dispatch_sync(dispatch_get_main_queue(), ^{
        next_drawable = [((__bridge CAMetalLayer*)metal_layer) nextDrawable];
    });
    auto ret = (__bridge CA::MetalDrawable*)next_drawable;
    ret->retain();
    return ret;
}


