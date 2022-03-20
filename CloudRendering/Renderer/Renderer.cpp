
//
//  Renderer.cpp
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-19.
//

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Renderer.hpp"
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>


#include <memory>

Renderer::Renderer() {
    device = Util::make_shared( MTL::CreateSystemDefaultDevice() );
    shader_library = Util::make_shared( device->newDefaultLibrary() );
    command_queue = Util::make_shared( device->newCommandQueue() );
}


MTL::Device* Renderer::get_device() {
    return device.get();
}

void Renderer::set_metal_layer( CA::MetalLayer* layer ) {
    metal_layer = layer;
}
