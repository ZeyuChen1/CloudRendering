
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
#include <iostream>
#include <vector>

/** initialize GPU resources */
Renderer::Renderer() {
    device = Util::rc(MTL::CreateSystemDefaultDevice());
    shader_library = Util::rc(device->newDefaultLibrary());
    command_queue = Util::rc(device->newCommandQueue());
    
    initialize_framebuffer_pipeline();
}


/** intialize the PSO for displaying a texture to the framebuffer */
void Renderer::initialize_framebuffer_pipeline() {
    auto pso_desc = Util::scoped(NEW(MTL::RenderPipelineDescriptor));
    
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


/** main render loop */
void Renderer::render_loop() {
    auto function_name = Util::ns_str("white_circle");
    NS::Error* err;
    auto fill_with_white = shader_library->newFunction(function_name);
    auto compute_pso = Util::rc(device->newComputePipelineState(fill_with_white, &err));
    MTL::Size threadgroup_size = MTL::Size::Make(compute_pso->threadExecutionWidth(), 8, 1);
    
    while (true) {
        CA::MetalDrawable* drawable = retrieve_next_drawable();
        
        auto fb_texture = drawable->texture();
        simd::uint2 viewport_size = { (uint32_t)fb_texture->width(), (uint32_t)fb_texture->height() };
        
        auto texture_desc = Util::scoped(NEW(MTL::TextureDescriptor));
        texture_desc->setTextureType(MTL::TextureType2D);
        texture_desc->setWidth(viewport_size.x);
        texture_desc->setHeight(viewport_size.y);
        texture_desc->setPixelFormat(MTL::PixelFormatRGBA32Float);
        texture_desc->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);
        auto texture = Util::scoped(device->newTexture(texture_desc.get()));
        
        
        auto command_buffer = Util::scoped(command_queue->commandBuffer());
        auto compute_cmd_encoder = command_buffer->computeCommandEncoder();
        compute_cmd_encoder->setComputePipelineState(compute_pso.get());
        compute_cmd_encoder->setBytes(&viewport_size, sizeof(simd::uint2), 0);
        compute_cmd_encoder->setTexture(texture.get(), 0);
        compute_cmd_encoder->dispatchThreads(MTL::Size::Make(viewport_size.x, viewport_size.y, 1), threadgroup_size);
        
        
        compute_cmd_encoder->endEncoding();
        compute_cmd_encoder->release();
        
        auto render_pass_desc = Util::scoped(NEW(MTL::RenderPassDescriptor));
        render_pass_desc->setRenderTargetWidth(viewport_size.x);
        render_pass_desc->setRenderTargetHeight(viewport_size.y);
        render_pass_desc->colorAttachments()->object(0)->setTexture(fb_texture);
        render_pass_desc->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        render_pass_desc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0, 0, 0, 0));
        render_pass_desc->setDefaultRasterSampleCount(1);
        
        
        auto render_command_encoder = Util::scoped(command_buffer->renderCommandEncoder(render_pass_desc.get()));
        render_command_encoder->setRenderPipelineState(framebuffer_pso.get());
        render_command_encoder->setVertexBuffer(quad_vertices.get(), 0, 0);
        render_command_encoder->setFragmentTexture(texture.get(), 0);
        render_command_encoder->setFragmentBytes(&viewport_size, sizeof(viewport_size), 0);
        render_command_encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, 0, 6, 1);
        render_command_encoder->endEncoding();
        
        command_buffer->commit();
        command_buffer->waitUntilCompleted();
        
        drawable->present();
        drawable->release();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

