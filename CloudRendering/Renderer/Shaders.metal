//
//  Shaders.metal
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-21.
//

#include <metal_stdlib>
#include "SharedTypes.h"

using namespace metal;

fragment float4 color() {
    return { 1.0f, 1.0f, 1.0f, 1.0f };
}


kernel void fill_with_white(uint2 tid [[ thread_position_in_grid ]],
                            constant int2& view_port_size [[ buffer(0) ]],
                            texture3d<float, access::write> voxels [[ texture(0) ]])
{
    
}
