// Utility
#pragma once
#include <memory>
#include <string>

namespace Util {

    // convenience function for using shared_ptr with Apple objects
    template <class T>
    inline std::shared_ptr<T> make_shared(T* ns_object) {
        return std::shared_ptr<T>( ns_object, [](T* obj) { obj->release(); } );
    }

}
