// Utility
#pragma once
#include <memory>
#include <string>
#include <Foundation/Foundation.hpp>

#define NEW(type) type::alloc()->init()

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



}
