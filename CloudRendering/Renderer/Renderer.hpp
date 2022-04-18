#pragma once

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>
#include <thread>
#include <vector>


class Renderer {
private:
    static constexpr uint32_t INTERNAL_RESOLUTION_WIDTH = 2048;
    static constexpr uint32_t INTERNAL_RESOLUTION_HEIGHT = 2048;
    
/// Basic GPU resources
    std::shared_ptr<MTL::Device> device;
    std::shared_ptr<MTL::Library> shader_library;
    std::shared_ptr<MTL::CommandQueue> command_queue;
    
    std::shared_ptr<MTL::RenderPipelineState> framebuffer_pso;
    std::shared_ptr<MTL::Buffer> quad_vertices;
    
    void initialize_framebuffer_pipeline();
    void draw_texture_to_screen(std::shared_ptr<MTL::CommandBuffer> command_buffer,
                                std::shared_ptr<CA::MetalDrawable> drawable,
                                std::shared_ptr<MTL::Texture> tex,
                                bool is_greyscale);
        
    
/// Swapchain
    CA::MetalLayer* metal_layer;
    CA::MetalDrawable* retrieve_next_drawable();
    
    
/// Perlin noise
    std::shared_ptr<MTL::ComputePipelineState> gen_density_pso;
    std::shared_ptr<MTL::Buffer> permutations_buffer;
    std::shared_ptr<MTL::Texture> cloud_density_map;
    
    std::shared_ptr<MTL::ComputePipelineState> gen_normal_pso;
    std::shared_ptr<MTL::Texture> cloud_normal_map;
    
    void initialize_cloud_generation_resources();
    void generate_cloud(std::shared_ptr<MTL::CommandBuffer>);
    
    
/// Skydome
    std::shared_ptr<MTL::RenderPipelineState> skydome_pso;
    size_t skydome_vertex_count;
    size_t skydome_index_count;
    std::shared_ptr<MTL::Buffer> skydome_vertices;
    std::shared_ptr<MTL::Buffer> skydome_indices;
    
    void initialize_skydome_pipeline();
    void draw_skydome(std::shared_ptr<MTL::CommandBuffer>, std::shared_ptr<MTL::Texture>);
    
    
/// Synchronization
    std::thread renderer_thread;
    void render_loop();
    
public:
    explicit Renderer();
    
    /** get Metal device */
    MTL::Device* get_device() { return device.get(); }
    
    /** assign core animation metal layer for drawable retrival */
    void set_metal_layer(CA::MetalLayer* layer) { metal_layer = layer; }

    /** start the render loop on a different thread */
    void start_render_loop() { renderer_thread = std::thread(&Renderer::render_loop, this); }
};


/**
 Helper function for loading files within app bundle
 */
std::string get_full_path(std::string const& path);
