// Utility
#pragma once
#include <memory>
#include <string>
#include <cmath>
#include <numbers>
#include <Foundation/Foundation.hpp>
#include <simd/simd.h>

namespace Util {

    // convenience function for using shared_ptr with Apple objects
    template <class T>
    inline std::shared_ptr<T> rc(T* ns_object) {
        return std::shared_ptr<T>(ns_object, [](T* obj) { obj->release(); });
    }
    
    inline NS::String* ns_str(const char* s) {
        return NS::String::alloc()->init(s, NS::UnicodeStringEncoding);
    }

    
    inline const char* c_str(NS::String* s) {
        return s->cString(NS::UnicodeStringEncoding);
    }
    
    // auto release NSObject when out of scope
    struct NSDeleter {
        void operator()(NS::Object* ns_object) {
            ns_object->release();
        }
    };
    
    template <class T>
    inline std::unique_ptr<T, NSDeleter> scoped(T* ns_object) {
        return std::unique_ptr<T, NSDeleter>(ns_object);
    }
    
    template <class _NSObject>
    inline std::unique_ptr<_NSObject, NSDeleter> new_scoped() {
        return std::unique_ptr<_NSObject, NSDeleter>(_NSObject::alloc()->init());
    }


}


namespace Math {
    using namespace simd;
    
    constexpr float pi = std::numbers::pi;
    
    inline float4x4 scale(float x) {
        float4x4 ret = float4x4(x);
        ret.columns[3][3] = 1.0f;
        return ret;
    }
    
    /**
     Look at matrix
     */
    inline float4x4 look_at(float3 eye, float3 at, float3 up) {
        float4x4 ret;
        
        float3 z = normalize(at - eye);
        float3 x = normalize(cross(up, z));
        float3 y = cross(z, x);
        
        ret.columns[0] = make_float4(x.x, y.x, z.x, 0.f);
        ret.columns[1] = make_float4(x.y, y.y, z.y, 0.f);
        ret.columns[2] = make_float4(x.z, y.z, z.z, 0.f);
        ret.columns[3] = make_float4(-dot(x, eye), -dot(y, eye), -dot(z, eye), 1.0f);
        
        return ret;
    }
    
    /**
     Perspective projection matrix
     */
    inline float4x4 perspective(float fovy, float aspect_ratio, float z_near, float z_far) {
        float y_scale = 1 / tanf(fovy * 0.5);
        float x_scale = y_scale / aspect_ratio;
        float z_scale = z_far / (z_far - z_near);
        
        float4x4 ret;
        ret.columns[0][0] = x_scale;
        ret.columns[1][1] = y_scale;
        ret.columns[2][2] = z_scale;
        ret.columns[2][3] = 1.0f;
        ret.columns[3][2] = -z_near * z_scale;
        
        return ret;
    }
    
    inline float radian(float degree) {
        return degree / 360.f * 2 * pi;
    }
}
