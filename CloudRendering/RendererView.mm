//
//  RendererView.m
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-19.
//

#import "RendererView.h"
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#include <memory>
#include "Renderer/Renderer.hpp"

@implementation RendererView
{
    CAMetalLayer* metalLayer;
    std::shared_ptr<Renderer> renderer;
}

- (id) init {
    self = [super init];
    
    metalLayer = [[CAMetalLayer alloc] init];
    renderer = std::make_shared<Renderer>();
    [metalLayer setDevice:(__bridge id<MTLDevice>) renderer->get_device()];
    renderer->set_metal_layer((__bridge CA::MetalLayer*) metalLayer);
    
    self.wantsLayer = YES;
    self.layer = metalLayer;
    return self;
}

- (void) beginRendering {
    
}

@end
