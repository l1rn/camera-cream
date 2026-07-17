#ifndef VERSION_HPP
#define VERSION_HPP

#include <string_view>

namespace Version {
    constexpr std::string_view FULL_STRING = "v1.0.0-0-ga1a2213-dirty";
    
    constexpr int MAJOR = 1;
    constexpr int MINOR = 0;
    constexpr int PATCH = 0;
    
    constexpr int COMMIT_COUNT = 0; 
    
    constexpr std::string_view GIT_HASH = "a1a2213";
}

#endif 
