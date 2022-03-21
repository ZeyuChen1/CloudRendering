
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Renderer.hpp"
#include "Util.hpp"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <dispatch/dispatch.h>

#include <memory>
#include <thread>

//constexpr MTL::PixelFormat DRAWABLE_PIXEL_FORMAT = MTL::PixelFormatBGRA8Unorm;

#define NEW(type) type::alloc()->init()

Renderer::Renderer() {
    device = Util::rc(MTL::CreateSystemDefaultDevice());
    shader_library = Util::rc(device->newDefaultLibrary());
    command_queue = Util::rc(device->newCommandQueue());
}


MTL::Device* Renderer::get_device() {
    return device.get();
}


void Renderer::set_metal_layer(CA::MetalLayer* layer) {
    metal_layer = layer;
}


void Renderer::start_render_loop() {
    renderer_thread = std::thread(&Renderer::render_loop, this);
}

void Renderer::render_loop() {
    auto function_name = Util::str("fill_with_white");
    NS::Error* err;
    auto fill_with_white = shader_library->newFunction(function_name);
    auto compute_pso = Util::rc(device->newComputePipelineState(fill_with_white, &err));
    
    while (true) {
        CA::MetalDrawable* drawable = retrieve_next_drawable();
        
        MTL::Texture* texture = drawable->texture();
        simd::uint2 viewport_size = { (uint32_t)texture->width(), (uint32_t)texture->height() };
        
        MTL::CommandBuffer* command_buffer = command_queue->commandBuffer();
        auto compute_cmd_encoder = command_buffer->computeCommandEncoder();
        compute_cmd_encoder->setComputePipelineState(compute_pso.get());
        compute_cmd_encoder->setBytes(&viewport_size, sizeof(simd::uint2), 0);
        compute_cmd_encoder->setTexture(texture, 0);
        compute_cmd_encoder->endEncoding();
        
        drawable->present();
        drawable->release();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
