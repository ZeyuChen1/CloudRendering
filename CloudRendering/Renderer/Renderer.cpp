
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Renderer.hpp"
#include "Util.hpp"
#include "ObjLoader.hpp"
#include "SharedTypes.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <dispatch/dispatch.h>

#include <memory>
#include <thread>
#include <iostream>
#include <vector>
#include <random>

/** initialize GPU resources */
Renderer::Renderer() {
    device = Util::rc(MTL::CreateSystemDefaultDevice());
    shader_library = Util::rc(device->newDefaultLibrary());
    command_queue = Util::rc(device->newCommandQueue());
    
    initialize_framebuffer_pipeline();
    initialize_cloud_generation_resources();
    initialize_skydome_pipeline();
}


/** intialize the PSO for displaying a texture to the framebuffer */
void Renderer::initialize_framebuffer_pipeline() {
    auto pso_desc = Util::new_scoped<MTL::RenderPipelineDescriptor>();
    
    // assign shaders
    auto vertex_shader = Util::scoped(shader_library->newFunction(Util::ns_str("vertex_passthrough")));
    auto frag_shader = Util::scoped(shader_library->newFunction(Util::ns_str("texture_passthrough")));
    pso_desc->setVertexFunction(vertex_shader.get());
    pso_desc->setFragmentFunction(frag_shader.get());
    
    // vertex out format
    auto vertex_desc = Util::scoped(MTL::VertexDescriptor::vertexDescriptor());
    
    // render targets
    pso_desc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    
    // miscellaneous
    pso_desc->setSampleCount(1);
    
    // create PSO
    NS::Error* err;
    MTL::RenderPipelineState* pso = device->newRenderPipelineState(pso_desc.get(), &err);
    if (err != nullptr)
        std::cerr << "Failed to create framebuffer pso: " << Util::c_str(err->description()) << std::endl;
    else
        framebuffer_pso = Util::rc(pso);
    
    // upload vertices data
    std::vector<simd::float3> vertices {
        simd::make_float3(-1.0f, 1.0f, 0.0f), simd::make_float3(-1.0f, -1.0f, 0.0f), simd::make_float3(1.0f, 1.0f, 0.0f),
        simd::make_float3(-1.0f, -1.0f, 0.0f), simd::make_float3(1.0f, -1.0f, 0.0f), simd::make_float3(1.0f, 1.0f, 0.0f)
    };
    quad_vertices = Util::rc(device->newBuffer(vertices.data(), sizeof(simd::float3) * vertices.size(), MTL::StorageModeManaged));
}


/**
 Initialize resouces for the skydome
 */
void Renderer::initialize_skydome_pipeline() {
    std::string dome_path = get_full_path("Assets/hemisphere.obj");
    ObjLoader hemisphere { dome_path };
    
    skydome_vertex_count = hemisphere.get_vertices_count();
    skydome_index_count = hemisphere.get_indices_count();
    
    skydome_vertices = Util::rc(device->newBuffer(hemisphere.get_vertices_data(),
                                                  hemisphere.get_vertices_data_size(),
                                                  MTL::StorageModeManaged));
    
    skydome_indices = Util::rc(device->newBuffer(hemisphere.get_indices_data(),
                                                 hemisphere.get_indices_data_size(),
                                                 MTL::StorageModeManaged));
    
    auto vertex_shader_name = Util::ns_str("transform");
    auto fragment_shader_name = Util::ns_str("draw_skydome");
    auto vertex_shader = Util::scoped(shader_library->newFunction(vertex_shader_name));
    auto fragment_shader = Util::scoped(shader_library->newFunction(fragment_shader_name));
    
    auto pso_desc = Util::new_scoped<MTL::RenderPipelineDescriptor>();
    pso_desc->setVertexFunction(vertex_shader.get());
    pso_desc->setFragmentFunction(fragment_shader.get());
    pso_desc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    pso_desc->setSampleCount(1);
    
    NS::Error* err;
    skydome_pso = Util::rc(device->newRenderPipelineState(pso_desc.get(), &err));
    
}


/**
 Draw textured skydome
 */
void Renderer::draw_skydome(std::shared_ptr<MTL::CommandBuffer> cmd_buffer,
                  std::shared_ptr<MTL::Texture> out_texture)
{
    float width = (float)out_texture->width();
    float height = (float)out_texture->height();
    
    auto rp_desc = Util::new_scoped<MTL::RenderPassDescriptor>();
    rp_desc->setRenderTargetWidth(out_texture->width());
    rp_desc->setRenderTargetHeight(out_texture->height());
    rp_desc->colorAttachments()->object(0)->setTexture(out_texture.get());
    rp_desc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    rp_desc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(0, 0, 0, 0));
    
    auto encoder = cmd_buffer->renderCommandEncoder(rp_desc.get());
    
    encoder->setRenderPipelineState(skydome_pso.get());
    encoder->setVertexBuffer(skydome_vertices.get(), 0, 0);
    encoder->setFrontFacingWinding(MTL::WindingClockwise);
    
    simd::float4x4 lookat = Math::look_at({ 0.f, 0.f, 0.0f },
                                          { 0, cos(Math::radian(45)), sin(Math::radian(45)) },
                                          { 0, 1, 0 });
    simd::float4x4 view = lookat * Math::scale(100.f);
    
//    simd::float4x4 view = Math::scale(5);
    
    encoder->setVertexBytes(&view, sizeof(simd::float4x4), 1);
    
    simd::float4x4 view_t_i = simd::transpose(simd::inverse(view));
    encoder->setVertexBytes(&view_t_i, sizeof(simd::float4x4), 2);
    
    simd::float4x4 proj = Math::perspective(Math::radian(100.f), width / height, 0.0f, 100.f);
//    simd::float4x4 proj = simd::float4x4(1.0f);
    
    encoder->setVertexBytes(&proj, sizeof(simd::float4x4), 3);
    
    encoder->setFragmentTexture(cloud_density_map.get(), 0);
    
    encoder->setFragmentTexture(cloud_density_map.get(), 0);
    
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle,
                                   skydome_index_count,
                                   MTL::IndexTypeUInt32,
                                   skydome_indices.get(), 0);
    encoder->endEncoding();
}

/**
 Initialize noise generation resources
 */
void Renderer::initialize_cloud_generation_resources() {
    /// For density map generation
    {
        auto shader_name = Util::scoped(Util::ns_str("generate_cloud_density_map"));
        auto shader = Util::scoped(shader_library->newFunction(shader_name.get()));
        NS::Error* err;
        gen_density_pso = Util::rc(device->newComputePipelineState(shader.get(), &err));
        
        // load GPU buffer
        std::uniform_real_distribution<float> dist { 0.f, 2 * Math::pi };
        std::default_random_engine rng;
        std::vector<float> p;
        
        for (int i = 0; i < 128; i += 1)
            p.push_back(dist(rng));
        
        
        permutations_buffer = Util::rc(device->newBuffer(p.data(), p.size() * sizeof(float), MTL::StorageModeManaged));
        
        auto cloud_density_map_desc = Util::new_scoped<MTL::TextureDescriptor>();
        cloud_density_map_desc->setWidth(INTERNAL_RESOLUTION_WIDTH);
        cloud_density_map_desc->setHeight(INTERNAL_RESOLUTION_HEIGHT);
        cloud_density_map_desc->setTextureType(MTL::TextureType2D);
        cloud_density_map_desc->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);
        cloud_density_map_desc->setPixelFormat(MTL::PixelFormatR32Float);
        cloud_density_map = Util::rc(device->newTexture(cloud_density_map_desc.get()));
    }
    
    /// For normal map generation
    {
        auto shader_name = Util::scoped(Util::ns_str("generate_normal_map"));
        auto shader = Util::scoped(shader_library->newFunction(shader_name.get()));
        NS::Error* err;
        gen_normal_pso = Util::rc(device->newComputePipelineState(shader.get(), &err));
        
        auto cloud_normal_map_desc = Util::new_scoped<MTL::TextureDescriptor>();
        cloud_normal_map_desc->setWidth(INTERNAL_RESOLUTION_WIDTH);
        cloud_normal_map_desc->setHeight(INTERNAL_RESOLUTION_HEIGHT);
        cloud_normal_map_desc->setTextureType(MTL::TextureType2D);
        cloud_normal_map_desc->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);
        cloud_normal_map_desc->setPixelFormat(MTL::PixelFormatRGBA8Snorm);
        cloud_normal_map = Util::rc(device->newTexture(cloud_normal_map_desc.get()));
    }
    
}


/**
 Encode noise generation command buffer
 */
void Renderer::generate_cloud(std::shared_ptr<MTL::CommandBuffer> command_buffer) {
    auto encoder = Util::scoped(command_buffer->computeCommandEncoder());
    auto dispatch_size = MTL::Size::Make(INTERNAL_RESOLUTION_WIDTH, INTERNAL_RESOLUTION_HEIGHT, 1);
    simd::uint2 dimension = { INTERNAL_RESOLUTION_WIDTH, INTERNAL_RESOLUTION_HEIGHT };
    
    /// Dispatch `generate_cloud_density_map`
    {
        encoder->setComputePipelineState(gen_density_pso.get());
        // texture dimension
        encoder->setBytes(&dimension, sizeof(simd::uint2), 0);
        // permutation array
        encoder->setBuffer(permutations_buffer.get(), 0, 1);
        // output texture
        encoder->setTexture(cloud_density_map.get(), 0);
        
        auto threadgroup_size = MTL::Size::Make(gen_density_pso->threadExecutionWidth(),
                                                gen_density_pso->threadExecutionWidth(), 1);
        encoder->dispatchThreads(dispatch_size, threadgroup_size);
    }
    
    /// Dispatch `generate_normal_map`
    {
        encoder->setComputePipelineState(gen_normal_pso.get());
        encoder->setBytes(&dimension, sizeof(simd::uint2), 0);
        encoder->setTexture(cloud_density_map.get(), 0);
        encoder->setTexture(cloud_normal_map.get(), 1);
        auto threadgroup_size = MTL::Size::Make(gen_normal_pso->threadExecutionWidth(),
                                                gen_normal_pso->threadExecutionWidth(), 1);
        encoder->dispatchThreads(dispatch_size, threadgroup_size);
    }
    
    encoder->endEncoding();
}


/**
 Draw texture to framebuffer
 */
void Renderer::draw_texture_to_screen(std::shared_ptr<MTL::CommandBuffer> command_buffer,
                                      std::shared_ptr<CA::MetalDrawable> drawable,
                                      std::shared_ptr<MTL::Texture> tex,
                                      bool is_greyscale)
{
    auto framebuffer_texture = drawable->texture();
    simd::uint2 viewport_size = { (uint32_t)framebuffer_texture->width(), (uint32_t)framebuffer_texture->height() };
    auto render_pass_desc = Util::new_scoped<MTL::RenderPassDescriptor>();
    render_pass_desc->setRenderTargetWidth(viewport_size.x);
    render_pass_desc->setRenderTargetHeight(viewport_size.y);
    render_pass_desc->colorAttachments()->object(0)->setTexture(framebuffer_texture);
    render_pass_desc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    render_pass_desc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0, 0, 0, 0));
    render_pass_desc->setDefaultRasterSampleCount(1);

    auto encoder = Util::scoped(command_buffer->renderCommandEncoder(render_pass_desc.get()));
    encoder->setRenderPipelineState(framebuffer_pso.get());
    encoder->setVertexBuffer(quad_vertices.get(), 0, 0);
    encoder->setFragmentTexture(tex.get(), 0);
    encoder->setFragmentBytes(&viewport_size, sizeof(viewport_size), 0);
    encoder->setFragmentBytes(&is_greyscale, sizeof(bool), 1);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, 0, 6, 1);
    encoder->endEncoding();

    command_buffer->presentDrawable(drawable.get());
}


/** main render loop */
void Renderer::render_loop() {
    while (true) {
        std::shared_ptr<CA::MetalDrawable> drawable = Util::rc(retrieve_next_drawable());
        std::shared_ptr<MTL::CommandBuffer> command_buffer = Util::rc(command_queue->commandBuffer());
        
        auto framebuffer_texture = Util::rc(drawable->texture());
        
        generate_cloud(command_buffer);
//        draw_texture_to_screen(command_buffer, drawable, cloud_density_map, true);
        draw_skydome(command_buffer, framebuffer_texture);
        
        command_buffer->presentDrawable(drawable.get());
        command_buffer->commit();
        command_buffer->waitUntilCompleted();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

