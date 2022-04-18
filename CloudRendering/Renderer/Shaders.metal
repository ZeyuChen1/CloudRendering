//
//  Shaders.metal
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-21.
//

#include <metal_stdlib>
#include "SharedTypes.h"

using namespace metal;

/**
 Draw given texture to the framebuffer
 */
vertex float4 vertex_passthrough(device float3 const* vertices [[ buffer(0) ]],
                                 uint vid [[ vertex_id ]])
{
    float3 v = vertices[vid];
    return float4(v, 1.0f);
}

fragment float4 texture_passthrough(float4 pos [[ position ]],
                                    constant uint2& viewport_size [[ buffer(0) ]],
                                    constant bool& is_greyscale [[ buffer(1) ]],
                                    texture2d<float, access::sample> tex [[ texture(0) ]])
{
    float2 uv = { pos.x / (float)viewport_size.x, pos.y / (float)viewport_size.y };
    constexpr sampler s{ coord::normalized, mag_filter::linear, min_filter::nearest };
    float4 texel = tex.sample(s, uv);
    if (is_greyscale) {
        return float4(float3(texel.x * 0.5f + 0.5f), 1.0f);
    } else {
        return float4(tex.sample(s, uv).xyz, 1.0f);
    }
}



/**
 Draw skydome
 */
struct VertexOut {
    float4 pos [[ position ]];
    float3 normal;
    float2 uv;
};

/**
 Simple vertex transform function
 */
vertex VertexOut transform(uint id                          [[ vertex_id ]],
                           device Vertex const* vertices    [[ buffer(0) ]],
                           constant float4x4& view          [[ buffer(1) ]],
                           constant float4x4& view_t_i      [[ buffer(2) ]],
                           constant float4x4& proj          [[ buffer(3) ]])
{
    VertexOut out;
    Vertex v = vertices[id];
    out.pos = proj * view * float4(v.position, 1.0f);
    out.uv = v.uv;
    out.normal = (view_t_i * float4(v.normal, 1.0f)).xyz;
    return out;
}

/**
 Render texture to skydome
 */
fragment float4 draw_skydome(VertexOut in                           [[ stage_in ]],
                             texture2d<float, access::sample> tex   [[ texture(0) ]])
{
    constexpr sampler s { min_filter::linear, mag_filter::linear, coord::normalized };
    float4 color = tex.sample(s, in.uv);
    color = float4(float3(color.x * 0.5 + 0.5), 1.0f);
    return color;
}



/**
 Gaussian bell function
 */
inline float gaussian(float x, float peak, float center, float width) {
    return peak * exp(-(x - center) * (x - center) / (2 * width * width));
}

/**
 Fall off function
 */
inline float fall_off(float x, float top_width) {
    if (x < top_width) {
        return 1.0;
    } else {
        return gaussian(x - top_width, 1.0, 0.0, 0.8 * (1.0 - top_width));
    }
}

/**
 Perlin's fade function
 */
inline float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float2 gradient(uint2 pos, constant float* perms) {
    uint i = (pos.x * 8 + pos.y) % 128;
    float theta = perms[i];
    return float2(cos(theta), sin(theta));
}

/**
 2D perlin noise
 */
float perlin_2D(float2 p, constant float* perms)
{
    float grid_size = 128.f;
    uint2 top_left      = uint2(floor(p / grid_size));
    uint2 top_right     = top_left + uint2(1, 0);
    uint2 bottom_left   = top_left + uint2(0, 1);
    uint2 bottom_right  = top_left + uint2(1, 1);
    
    float2 aa = gradient(top_left, perms);
    float2 ab = gradient(top_right, perms);
    float2 ba = gradient(bottom_left, perms);
    float2 bb = gradient(bottom_right, perms);
    
    float x = fmod(p.x, grid_size) / grid_size;
    float y = fmod(p.y, grid_size) / grid_size;
    float2 xy = float2(x, y);
    
    float top = mix(dot(aa, xy), dot(ab, xy - float2(1.f, 0.f)), fade(x));
    float bot = mix(dot(ba, xy - float2(0.f, 1.f)), dot(bb, xy - float2(1.f, 1.f)), fade(x));
    float result = mix(top, bot, fade(y));
    
    return result;
}


/**
 Generate cloud dnesity map using 2D perlin noise
 */
kernel void generate_cloud_density_map(uint2 pos             [[ thread_position_in_grid ]],
                                       constant uint2& dim   [[ buffer(0) ]],
                                       constant float* p     [[ buffer(1) ]],
                                       texture2d<float, access::write> out [[ texture(0) ]])
{
    constexpr uint octave = 8;
    float sum = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    
    for(uint i = 0; i < octave; i += 1) {
        sum += perlin_2D((float2(pos) + 0.5) * frequency, p) * amplitude;
        amplitude *= 0.5;
        frequency *= 2.0;
    }

    float result = gaussian(sum, 1.0f, 1.0f, .6f);
    
    float2 center_point = float2(dim) / 2.f;
    float distance_to_center = distance(center_point, float2(pos)) / float(dim.x / 2);
    
    result *= fall_off(distance_to_center, 0.8);
    
    out.write(float4(result, 0, 0, 0), pos);
}


/**
 Generate normal map based on a height map using Sobel filter
 */
kernel void generate_normal_map(uint2           pos [[ thread_position_in_grid ]],
                                constant uint2& dim [[ buffer(0) ]],
                                texture2d<float, access::sample> height_map [[ texture(0) ]],
                                texture2d<float, access::write> out         [[ texture(1) ]])
{
    // Sobel filter
    thread float p[3][3];
    for (int x_offset = -1; x_offset <= 1; x_offset += 1) {
        for (int y_offset = -1; y_offset <= 1; y_offset += 1) {
            int x = pos.x + x_offset, y = pos.y + y_offset;
            if (!(x >= int(dim.x) || x < 0 || y >= int(dim.y) || y < 0))
                p[x_offset + 1][y_offset + 1] = height_map.read(uint2(x, y)).x;
        }
    }
    
    float3 n;
    n.x = -(p[2][2] - p[0][2] + 2 * (p[2][1] - p[0][1]) + p[2][0] - p[0][0]);
    n.y = -(p[0][0] - p[2][0] + 2 * (p[1][0] - p[1][2]) + p[2][0] - p[2][2]);
    n.z = 1.0f;
    
    out.write(float4(normalize(n), 0.0f), pos);
}


float relative_luminance(float zeta, float gamma) {
    float a = 1.1f;
    float b = 1.0f;
    float c = 1.0f;
    float d = 1.0f;
    float e = 0.7f;
    
    return (1 + a * exp(b / cos(zeta))) * (1 + c * exp(d * gamma) + e * cos(gamma) * cos(gamma));
}


/**
 Draw shaded skydome
 */
kernel void shade_sky(uint2 tid [[ thread_position_in_grid ]],
                      constant uint2& dim           [[ buffer(0) ]],
                      constant SunParameters& sun   [[ buffer(1) ]],
                      texture2d<float, access::sample> cloud_density_map  [[ texture(0) ]],
                      texture2d<float, access::sample> normal_map         [[ texture(1) ]])
{
//    float2 center = float2(dim) / 2.0;
//    float2 pos = (float2(tid) - center) / float(dim.x / 2);
//    float3 view = normalize({ pos.x, pos.y,  });
    
}
