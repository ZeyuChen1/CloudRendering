#pragma once

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>
#include <thread>


class Renderer {
private:
// GPU resources
    std::shared_ptr<MTL::Device> device;
    std::shared_ptr<MTL::Library> shader_library;
    std::shared_ptr<MTL::CommandQueue> command_queue;
    
// display
    CA::MetalLayer* metal_layer;

    simd::float2 viewport_size;
    
// synchronization
    std::thread renderer_thread;
    
    CA::MetalDrawable* retrieve_next_drawable();
    void render_loop();
    
public:
    explicit Renderer();
    MTL::Device* get_device();
    void set_metal_layer(CA::MetalLayer* metal_layer);
    void start_render_loop();
};
