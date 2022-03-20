#pragma once

#include "Common.hpp"
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>


class Renderer {
private:
// GPU resources
    std::shared_ptr<MTL::Device> device;
    std::shared_ptr<MTL::Library> shader_library;
    std::shared_ptr<MTL::CommandQueue> command_queue;
    
// display
    CA::MetalLayer* metal_layer;
    
    MTL::Drawable* retrieve_next_drawable();
    
public:
    explicit Renderer();
    MTL::Device* get_device();
    void set_metal_layer( CA::MetalLayer* metal_layer );
};
