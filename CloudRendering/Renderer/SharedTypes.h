#pragma once
#include <simd/simd.h>

typedef struct vertex_t {
    simd_float3 position;
    simd_float2 uv;
    simd_float3 normal;
} Vertex;

struct SunParameters {
    simd_float3 position;
    float       light_intensity;
};
