#ifndef VERSION_HPP
#define VERSION_HPP

#include <string_view>

namespace Version {
    constexpr std::string_view FULL_STRING = "v1.0.0-1-g557503f-dirty";
    
    constexpr int MAJOR = 1;
    constexpr int MINOR = 0;
    constexpr int PATCH = 0;
    
    constexpr int COMMIT_COUNT = 1; 
    
    constexpr std::string_view GIT_HASH = "557503f";
}

#endif 
