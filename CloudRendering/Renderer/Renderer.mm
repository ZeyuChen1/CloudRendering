// why aren't these in metal-cpp???
#include "Renderer.hpp"
#import <QuartzCore/QuartzCore.h>
#include <string>

CA::MetalDrawable* Renderer::retrieve_next_drawable() {
    __block id<CAMetalDrawable> next_drawable;
    dispatch_sync(dispatch_get_main_queue(), ^{
        next_drawable = [(__bridge CAMetalLayer*)metal_layer nextDrawable];
    });
    auto ret = (__bridge CA::MetalDrawable*)next_drawable;
    ret->retain();
    return ret;
}


std::string get_full_path(std::string const& path) {
    NSString* relative_path = [[NSString alloc] initWithUTF8String:path.c_str()];
    NSString* resource_path = [[NSBundle mainBundle] resourcePath];
    NSString* fullPath = [[resource_path stringByAppendingString:@"/"] stringByAppendingString:relative_path];
    return std::string([fullPath cStringUsingEncoding:NSUTF8StringEncoding]);
}
