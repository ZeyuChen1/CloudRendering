#pragma once

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>
#include <thread>
#include <vector>


class Renderer {
private:
/* Basic GPU resources */
    std::shared_ptr<MTL::Device> device;
    std::shared_ptr<MTL::Library> shader_library;
    std::shared_ptr<MTL::CommandQueue> command_queue;
    
    std::shared_ptr<MTL::RenderPipelineState> framebuffer_pso;
    std::shared_ptr<MTL::Buffer> quad_vertices;
    
/* Swapchain */
    CA::MetalLayer* metal_layer;
    
/* Synchronization */
    std::thread renderer_thread;
    
/* Functions */
    CA::MetalDrawable* retrieve_next_drawable();
    void render_loop();
    void initialize_framebuffer_pipeline();
    
public:
    explicit Renderer();
    
    /** get Metal device */
    MTL::Device* get_device() { return device.get(); }
    
    /** assign core animation metal layer for drawable retrival */
    void set_metal_layer(CA::MetalLayer* layer) { metal_layer = layer; }
    
    /** start the render loop on a different thread */
    void start_render_loop() { renderer_thread = std::thread(&Renderer::render_loop, this); }
};
