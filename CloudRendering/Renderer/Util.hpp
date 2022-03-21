// Utility
#pragma once
#include <memory>
#include <string>
#include <Foundation/Foundation.hpp>

namespace Util {

// convenience function for using shared_ptr with Apple objects
template <class T>
inline std::shared_ptr<T> rc(T* ns_object) {
    return std::shared_ptr<T>( ns_object, [](T* obj) { obj->release(); } );
}

inline NS::String* str(const char* s) {
    return NS::String::alloc()->init(s, NS::UnicodeStringEncoding);
}

struct NSDeleter {
    void operator()(NS::Object* obj) { obj->release(); }
};

template <class T>
using Scoped = std::unique_ptr<T, NSDeleter>;
    
}
