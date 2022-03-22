//
//  Shaders.metal
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-21.
//

#include <metal_stdlib>
#include "SharedTypes.h"

using namespace metal;


vertex float4 vertex_passthrough(device float3 const* vertices [[ buffer(0) ]],
                                 uint vid [[ vertex_id ]])
{
    float3 v = vertices[vid];
    return float4(v, 1.0f);
}


fragment float4 texture_passthrough(float4 pos [[ position ]],
                                    constant uint2& viewport_size [[ buffer(0) ]],
                                    texture2d<float, access::sample> tex [[ texture(0) ]])
{
    float2 uv = { pos.x / (float)viewport_size.x, pos.y / (float)viewport_size.y };
    constexpr sampler s{ coord::normalized, filter::nearest };
    return float4(tex.sample(s, uv).xyz, 1.0f);
}


kernel void white_circle(uint2 tid [[ thread_position_in_grid ]],
                         constant uint2& viewport_size [[ buffer(0) ]],
                         texture2d<float, access::write> tex [[ texture(0) ]])
{
    if (tid.x > viewport_size.x || tid.y > viewport_size.y) return;
    
    float2 uv = { (float)tid.x / (float)viewport_size.x, (float)tid.y / (float)viewport_size.y };
    
    if (length(uv) < 1) {
        tex.write(float4(length(uv)), tid);
    }
}
